/* @file Sink.hpp */

#pragma once

#include "AbstractSink.hpp"
#include "AbstractSource.hpp"
#include "PolyAdapterSink.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/indentlog/print/time.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/cxxutil/demangle.hpp"
#include <typeinfo>

namespace xo {
    namespace reactor {
        /* Sink for events of type T
         *
         * inheritance:
         *   ref::Refcount
         *     ^
         *     isa
         *     |
         *   reactor::AbstractEventProcessor
         *     ^
         *     isa
         *     |
         *   reactor::AbstractSink
         *     ^
         *     isa
         *     |
         *   reactor::Sink1<T>
         */
        template<typename T>
        class Sink1 : public AbstractSink {
        public:
            using Reflect = reflect::Reflect;
            using TypeDescr = reflect::TypeDescr;

        public:
            /* convenience:  convert abstract sink to Sink1<T>*,
             * or throw
             */
            static ref::rp<Sink1<T>> require_native(std::string_view caller,
                                                    ref::rp<AbstractSink> const & sink)
                {
                    using xo::scope;
                    using xo::xtag;

                    /* 1. if sink expects events of type T,
                     *    make direct connection
                     */
                    Sink1<T> * native_sink = nullptr;

                    native_sink = dynamic_cast<Sink1<T> *>(sink.get());

                    if (native_sink)
                        return native_sink;

                    /* 2. if sink is polymorphic,
                     *    make type-erasing adapter
                     */

                    if (sink->allow_polymorphic_source()) {
#ifdef DEBUG_NOT_USING
                        scope lscope("Sink1<T>::require_native: create PolyAdapterSink");
                        lscope.log(xtag("caller", caller));
#endif

                        return PolyAdapterSink<T>::make(sink);
                    }

                    if (!native_sink) {
#ifdef DEBUG_EVENT_TYPEINFO
                        std::type_info const * sink_parent_typeinfo
                            = sink->parent_typeinfo();
#endif

                        std::size_t src_hashcode = typeid(T).hash_code();

                        throw std::runtime_error(tostr("Sink1<T>::require_native"
                                                       ": wanted to sink S,  but sink expects T",
                                                       xtag("caller", caller),
                                                       xtag("T", sink->sink_ev_type()->canonical_name()),
                                                       xtag("S", reflect::type_name<T>()),
                                                       xtag("required_hashcode", typeid(Sink1<T>).hash_code()),
                                                       xtag("required_name", typeid(Sink1<T>).name()),
                                                       xtag("src_hashcode", src_hashcode),
                                                       xtag("sink_hashcode", sink->sink_ev_type()->native_typeinfo()->hash_code())
#ifdef DEBUG_EVENT_TYPEINFO
                                                       , xtag("sink_hashcode", sink->item_typeinfo()->hash_code())
                                                       , xtag("sink_parent_hashcode", sink_parent_typeinfo->hash_code())
                                                       , xtag("sink_parent_name", sink_parent_typeinfo->name())
                                                       , xtag("sink.type", sink->self_typename())
                                                       , xtag("sink.parent_type", sink->parent_typename())
#endif
                                                     ));
                    }

                    return native_sink;
                } /*require_native*/

            virtual TypeDescr sink_ev_type() const override { return reflect::Reflect::require<T>(); }
            /* accept incoming event */
            virtual void notify_ev(T const & ev) = 0;

            /* invoke these when this sink added to, or removed from, a source */
            virtual void notify_add_callback() {}
            virtual void notify_remove_callback() {}

            // ----- inherited from AbstractSink -----

            /* Sink1<T> only allows source providing T */
            virtual bool allow_polymorphic_source() const override { return false; }

            virtual void attach_source(ref::rp<AbstractSource> const & src) override {
                src->attach_sink(this);
            } /*attach_source*/

            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override {
                using xo::xtag;

                T * p_ev = ev_tp.recover_native<T>();

                if (p_ev) {
                    this->notify_ev(*p_ev);
                } else {
                    throw std::runtime_error(tostr("Sink1<T>::notify_ev_tp"
                                                   ": unable to convert ev_tp to T",
                                                   xtag("ev_tp.type", ev_tp.td()->canonical_name()),
                                                   xtag("T", reflect::type_name<T>())));
                }
            } /*notify_ev_tp*/
        }; /*Sink1*/

        /* a sink with no further downstream processors */
        template<typename T>
        class SinkEndpoint : public Sink1<T> {
        public:
            // ----- Inherited from AbstractEventProcessor -----

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { name_ = x; }

            virtual void visit_direct_consumers(std::function<void (ref::brw<AbstractEventProcessor>)> const &) override {
                /* *this is not an event source */
            } /*visit_direct_consumers*/

        private:
            /* reporting name for this sink */
            std::string name_;
        }; /*SinkEndpoint*/

        template<typename T, typename Fn>
        class SinkToFunction : public SinkEndpoint<T> {
        public:
            SinkToFunction(Fn fn) : fn_{std::move(fn)} {}

            /* NOTE: conservative choice here,  could templatize on this */
            virtual bool allow_volatile_source() const override { return false; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void notify_ev(T const & ev) override {
                ++(this->n_in_ev_);
                fn_(ev);
            } /*notify_ev*/

            virtual void display(std::ostream & os) const override {
                using xo::xtag;

                os << "<SinkToFunction"
                   << xtag("name", this->name())
                   << xtag("n_in_ev", this->n_in_ev())
                   << ">";
            } /*display*/

        private:
            Fn fn_;
            /* counts lifetime #of incoming events (see .notify_ev()) */
            uint32_t n_in_ev_ = 0;
        }; /*SinkToFunction*/

        /* sink that prints to console */
        template<typename T>
        class SinkToConsole : public SinkEndpoint<T> {
        public:
            SinkToConsole() {}

            virtual bool allow_volatile_source() const override { return true; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void notify_ev(T const & ev) override {
                //using logutil::operator<<;

                ++(this->n_in_ev_);

                std::cout << ev << std::endl;
            } /*notify_ev*/

            virtual void display(std::ostream & os) const override {
                using xo::xtag;

                os << "<SinkToConsole"
                   << xtag("name", this->name())
                   << xtag("n_in_ev", this->n_in_ev())
                   << ">";
            } /*display*/

        private:
            /* reporting name for this sink */
            std::string name_;
            /* counts lifetime #of incoming events (see .notify_ev()) */
            uint32_t n_in_ev_ = 0;
        }; /*SinkToConsole*/

#ifdef NOT_USING
        class TemporaryTest {
        public:
            static ref::rp<SinkToConsole<std::pair<xo::time::utc_nanos, double>>> realization_printer();
        }; /*TemporaryTest*/
#endif
    } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.hpp */
