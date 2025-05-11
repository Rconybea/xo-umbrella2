/* @file EventStore.hpp */

#pragma once

#include "Reducer.hpp"
#include "EventTimeFn.hpp"
#include "Sink.hpp"
#include "xo/webutil/HttpEndpointDescr.hpp"
#include "xo/printjson/PrintJson.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/ordinaltree/RedBlackTree.hpp"

namespace xo {
    namespace reactor {

        /* abstract event store api */
        class AbstractEventStore : virtual public ref::Refcount {
        public:
            using PrintJson = xo::json::PrintJson;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using HttpEndpointDescr = xo::web::HttpEndpointDescr;
            using Alist = xo::web::Alist;

        public:
            /* true iff .size() == 0 */
            virtual bool empty() const = 0;

            /* #of events currently held in this store */
            virtual std::uint32_t size() const = 0;

            /* TODO:
             *   1. TaggedGptr = discriminated union of
             *        a. TaggedRcptr (i.e. refcounted semantics)
             *        b. Unique      (i.e. unique_ptr semantics)
             *        c. Exptr       (i.e. unowned_ptr semantics)
             *        d. compact     (special-case -- value fits in pointer)
             *      will need mpl copy/assign stuff for TaggedUnique
             *   2. provide .last_n(), .last_dt()
             */

            virtual void http_snapshot(rp<PrintJson> const & pjson,
                                       std::ostream * p_os) const = 0;

            /* http endpoint; generates http output for this eventstore */
            virtual HttpEndpointDescr http_endpoint_descr(rp<PrintJson> const & pjson,
                                                          std::string const & url_prefix) const {

                /* important that lambda contains its own rp<PrintJson>;
                 * reference to stack will not do
                 */
                rp<PrintJson> pjson_rp = pjson;

                auto http_fn = ([this, pjson_rp]
                                (std::string const & /*uri*/,
                                 Alist const & /*alist*/,
                                 std::ostream * p_os)
                    {
                        /* WARNING: race condition here,
                         *          given webserver runs from a separate thread
                         */

                        this->http_snapshot(pjson_rp, p_os);
                    });

                return HttpEndpointDescr(url_prefix + "/snap", http_fn);
            } /*http_endpoint_descr*/

            virtual void clear() = 0;

            virtual void insert_tp(TaggedPtr const & ev_tp) = 0;
        }; /*AbstractEventStore*/

        /* in-memory storage for a set of events.
         *
         * Require:
         * - Event is null-constructible
         * - Event is copyable
         * - EventTimeFn :: Event -> utc_nanos
         *
         * inheritance
         *   ref::Refcount
         *     ^
         *     isa
         *     |
         *   reactor::AbstractEventProcessor       + req .visit_direct_consumers()
         *     ^
         *     isa
         *     |
         *   reactor::AbstractSink                 + req .sink_ev_type(), .notify_ev() etc.
         *     ^
         *     isa
         *     |
         *   reactor::Sink1<Event>                 + .attach_source(), .sink_ev_type(),
         *     ^                                     req .notify_ev() etc
         *     |
         *     isa
         *     |
         *   reactor::SinkEndpoint<Event>          + impl .visit_direct_consumers()
         *     ^
         *     isa
         *     |
         *   reactor::StructEventStore<Event, ..>  + .last_n() .last_dt() etc.
         */
        template<typename Event,
                 typename EventTimeFn>
        class EventStoreImpl : public SinkEndpoint<Event>,
                               public AbstractEventStore,
            ReducerBase<Event, EventTimeFn>
        {
            static_assert(EventTimeConcept<Event, EventTimeFn>);

        public:
            using utc_nanos = xo::time::utc_nanos;
            using nanos = xo::time::nanos;
            using EventTree = xo::tree::RedBlackTree<utc_nanos, Event,
                                                     xo::tree::OrdinalReduce<Event>>;
            using PrintJson = xo::json::PrintJson;
            using Alist = xo::web::Alist;
            using HttpEndpointDescr = xo::web::HttpEndpointDescr;

            static rp<EventStoreImpl> make() { return new EventStoreImpl(); }

            /* visit most recent n events in this store.
             * returns #of events actually visited
             *
             * if events visited are e1 .. en,  then:
             * (1) en is the most recent recorded event
             *     (.event_tm(en) is .tree.max_key())
             * (2) there are no events between e(i) and e(i+1)
             *     (i.e. visit does not skip over any events)
             * (3) if v < n,  then v = .size(),
             *     where v is the #of events visited
             *
             * require:
             * - Fn :: (Event -> )
             */
            template <typename Fn>
            std::uint32_t visit_last_n(std::uint32_t n, Fn && fn) const {
                std::uint32_t z = this->size();
                std::uint32_t lo = ((n >= z) ? 0 : z - n);

                typename EventTree::const_iterator lo_ix = this->tree_.find_ith(lo);
                typename EventTree::const_iterator hi_ix = this->tree_.cend();

                return this->visit_range(lo_ix, hi_ix, fn);
            } /*visit_last_n*/

            /* visit suffix of events sufficient to cover interval of length dt.
             * visit events in increasing timestamp order.
             *
             * if events visited are e1 .. en,  then:
             * (1) en is the most recent recorded event
             *     (.event_tm(en) is .tree.max_key())
             * (2) there are no events between e(i) and e(i+1)
             *     (i.e. visit does not skip over any events)
             * (3) if .event_tm(en) - .event_tm(e1) < dt,
             *     then e1 is the earliest recorded event
             *     (.event_tm(e1) is .tree.min_key())
             * (4) if .event_tm(en) - .event_tm(e1) > dt,
             *     then (.event_tm(en) - .event_tm(e2)) < dt
             *
             *        |<---------- dt ----------->|
             *      ^      ^                      ^
             *     e1     e2                     en
             */
            template <typename Fn>
            std::uint32_t visit_last_dt(nanos dt, Fn && fn) const {
                if (tree_.empty())
                    return 0;

                /* tree not empty -> has max key */
                utc_nanos tn = this->tree_.max_key();
                utc_nanos tk = tn - dt;

                typename EventTree::const_iterator lo_ix = this->tree_.find_glb(tk, true /*closed*/);
                typename EventTree::const_iterator hi_ix = this->tree_.end();

                return this->visit_range(lo_ix, hi_ix, fn);
            } /*visit_last_dt*/

            std::vector<Event> last_n(std::uint32_t n) const {
                std::vector<Event> retval;

                auto fn = [&retval](Event const &ev) { retval.push_back(ev); };

                this->visit_last_n(n, fn);

                return retval;
            } /*last_n*/

            std::vector<Event> last_dt(nanos dt) const {
                std::vector<Event> retval;

                auto fn = [&retval](Event const &ev) { retval.push_back(ev); };

                this->visit_last_dt(dt, fn);

                return retval;
            } /*last_dt*/

            void insert(Event const & ev) { this->tree_.insert(typename EventTree::value_type(this->event_tm(ev), ev)); }

            // ----- Inherited from AbstractEventStore -----

            virtual bool empty() const override { return tree_.empty(); }
            virtual std::uint32_t size() const override { return tree_.size(); }

            /* write http snapshot of current state to *p_os */
            virtual void http_snapshot(rp<PrintJson> const & pjson, std::ostream * p_os) const override {
                using xo::reflect::Reflect;

                /* visit last 100 events;
                 * write them to *p_os in increasing time order
                 */
                auto ev_v = this->last_n(100);

                pjson->print_tp(Reflect::make_tp(&ev_v), p_os);
            } /*http_snapshot*/

            virtual void clear() override { this->tree_.clear(); }

            virtual void insert_tp(TaggedPtr const & ev_tp) override {
                using xo::xtag;

                Event * p_ev = ev_tp.recover_native<Event>();

                if (p_ev) {
                    this->insert(*p_ev);
                } else {
                    throw std::runtime_error(tostr("StructEventStore<Event>::insert_tp"
                                                   ": unable to convert ev_tp to Event",
                                                   xtag("ev_tp.type", ev_tp.td()->canonical_name()),
                                                   xtag("Event", reflect::type_name<Event>())));
                }
            } /*insert_tp*/

            // ----- Inherited from AbstractSink -----

            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual bool allow_volatile_source() const override { return false; }
            virtual void notify_ev(Event const & ev) override {
                ++(this->n_in_ev_);
                this->insert(ev);
            }

            // ----- Inherited from AbstractSource -----

            virtual void display(std::ostream & os) const override {
                using xo::xtag;

                os << "<EventStoreImpl"
                   << xtag("name", this->name())
                   << xtag("n_in_ev", this->n_in_ev())
                   << ">";
            } /*display*/

            // ----- Inherited from AbstractEventProcessor -----

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { name_ = x; }

        private:
            EventStoreImpl() = default;

            template <typename Fn>
            std::uint32_t visit_range(typename EventTree::const_iterator lo_ix,
                                      typename EventTree::const_iterator hi_ix,
                                      Fn && fn) const {
                std::uint32_t n = 0;
                for (; lo_ix != hi_ix; ++lo_ix, ++n) {
                    fn(lo_ix->second);
                }

                return n;
            } /*visit_range*/

        private:
            /* reporting name for this store */
            std::string name_;
            /* fetches per-event timestamp */
            EventTimeFn event_tm_fn_;
            /* counts lifetime #of incoming events (see .notify_ev()) */
            uint32_t n_in_ev_ = 0;
            /* events stored here */
            EventTree tree_;
        }; /*EventStoreImpl*/

        template<typename Event>
        using StructEventStore = EventStoreImpl<Event, StructEventTimeFn<Event>>;

        template<typename Event>
        using PtrEventStore = EventStoreImpl<Event, PtrEventTimeFn<Event>>;

        /* Require:
         *   EventTimeConcept<T, StructEventTimeFn<T>>
         */
        template <typename T>
        class SinkToEventStore : public SinkEndpoint<T> {
        public:
            using EventStore = StructEventStore<T>;

        public:
            SinkToEventStore() = default;

            virtual void notify_ev(T const & ev) override {
                store_.insert(ev);
            } /*notify_ev*/

        private:
            /* stash remembered events (all of them!) here */
            EventStore store_;
        }; /*SinkToEventStore*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end EventStore.hpp */
