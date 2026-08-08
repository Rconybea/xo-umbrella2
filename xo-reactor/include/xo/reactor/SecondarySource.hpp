/* @file SecondarySource.hpp */

#pragma once

#include "EventSource.hpp"
#include "HeapReducer.hpp"
#include "Reactor.hpp"
#include "Sink.hpp"
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/callback/CallbackSet.hpp>
#include <xo/cxxutil/demangle.hpp>
#include <vector>

/* NB xo::pp names are QUALIFIED throughout this header, not brought in by
 * using-declarations.  Two reasons:
 *   - a using-decl at namespace scope in a public header leaks into every
 *     consumer's scope;
 *   - a function-local one is not enough either: ADL adds legacy xo::xtag
 *     whenever an argument type lives in namespace xo, and merges it into
 *     the candidate set.  Only a qualified call avoids that.
 */

namespace xo {
    namespace reactor {
        /* A passive event source.
         * Can use as backend publisher when implementating another
         * event processor.
         */
        template<typename Event, typename Reducer = HeapReducer<Event>>
        class SecondarySource : public EventSource<Sink1<Event>> {
        public:
            using EventSink = Sink1<Event>;
            template<typename Fn>
            using RpCallbackSet = fn::RpCallbackSet<Fn>;
            using CallbackId = fn::CallbackId;
            using TypeDescr = xo::reflect::TypeDescr;
            using utc_nanos = xo::time::utc_nanos;

        public:
            ~SecondarySource() = default;

            static rp<SecondarySource> make() { return new SecondarySource(); }

            /* last event delivered from this source --
             * i.e. event in most recent call to .deliver_one_aux()
             */
            Event const & last_annexed_ev() const { return this->reducer_.last_annexed_ev(); }

            void notify_upstream_exhausted() { this->upstream_exhausted_ = true; }

            /* make event available to reactor, by adding to internal reducer */
            void notify_secondary_event(Event const & ev) {
                /* test if ev is priming, update .current_tm */
                bool is_priming = this->preprocess_secondary_event(ev);

                this->reducer_.include_event(ev);

                this->postprocess_secondary_event(is_priming);
            } /*notify_secondary_event*/

            void notify_secondary_event(Event && ev) {
                bool is_priming = this->preprocess_secondary_event(ev);

                this->reducer_.include_event(ev);

                this->postprocess_secondary_event(is_priming);
            } /*notify_secondary_event*/

            template<typename T>
            void notify_secondary_event_v(T const & v) {

                if (v.empty())
                    return;

                xo::pp::scope log(XO_DEBUG_(this->debug_sim_flag_));

                log && log(xo::pp::xtag("name", this->name()));

                if (this->upstream_exhausted_) {
                    throw std::runtime_error("SecondarySource::notify_secondary_event_v"
                                             ": not allowed after upstream exhausted");
                }

                uint32_t n_ev = 0;

                for (Event const & ev : v) {
                    utc_nanos evtm = this->reducer_.event_tm(ev);

                    if (this->current_tm_ < evtm)
                        this->current_tm_ = evtm;

                    ++n_ev;
                }

                log && log(xo::pp::xtag("T", reflect::type_name<T>()),
                           xo::pp::xtag("n_ev", n_ev));

                if (n_ev > 0) {
                    /* if reducer is empty when .notify_secondary_event_v() begins,
                     * then reactor/simulator needs to be notified that source is no longer empty
                     */
                    bool is_priming = this->reducer_.is_empty();

                    for (Event const & ev : v)
                        this->reducer_.include_event(ev);

                    Reactor * reactor = this->parent_reactor_;

                    if (reactor) {
                        if (is_priming) {
                            /* reactor/simulator takes responsibility for delivering events */
                            reactor->notify_source_primed(bp<ReactorSource>::from_native(this));
                        }
                    } else {
                        /* special case if no reactor:  deliver immediately */

                        //this->deliver_one();
                        this->deliver_all();
                    }
                }
            } /*notify_secondary_event_v*/

            // ----- inherited from EventSource -----

            virtual CallbackId add_callback(rp<EventSink> const & cb) override {
                return this->cb_set_.add_callback(cb);
            } /*add_callback*/

            virtual void remove_callback(CallbackId id) override {
                this->cb_set_.remove_callback(id);
            } /*remove_callback*/

            // ----- inherited from ReactorSource -----

            virtual bool is_empty() const override { return this->reducer_.is_empty(); }
            virtual bool is_exhausted() const override { return this->upstream_exhausted_ && this->is_empty(); }

            virtual utc_nanos sim_current_tm() const override {

                if (this->reducer_.is_empty()) {
                    /* this is a tricky case.
                     * it means this source doesn't
                     * _know_ specific next event yet;  however new events
                     * may appear at any time by way of .notify_event()
                     *
                     * If event doesn't know next event,  then .current_tm isn't useful
                     * for establishing priority relative to other sources.
                     * rely on priming mechanism instead,
                     * which means that control should never come here.
                     */
                    return this->current_tm_;
                } else {
                    xo::pp::scope log(XO_DEBUG_(false /*this->debug_sim_flag_*/),
                              xo::pp::xtag("name", this->name_),
                              xo::pp::xtag("next_tm", this->reducer_.next_tm()));

                    return this->reducer_.next_tm();
                }
            } /*sim_current_tm*/

            virtual std::uint64_t deliver_one() override {
                return this->deliver_one_aux(true /*replay_flag*/);
            }

            virtual std::uint64_t sim_advance_until(utc_nanos target_tm,
                                                    bool replay_flag) override
                {
                    uint64_t retval = 0;

                    while (!this->reducer_.is_empty()) {
                        utc_nanos tm = this->sim_current_tm();

                        if (tm < target_tm) {
                            retval += this->deliver_one_aux(replay_flag);
                        } else {
                            break;
                        }
                    }

                    return retval;
                } /*sim_advance_until*/

            virtual void notify_reactor_add(Reactor * reactor) override {
                assert(!this->parent_reactor_);

                this->parent_reactor_ = reactor;
            } /*notify_reactor_add*/

            virtual void notify_reactor_remove(Reactor * /*reactor*/) override {}

            // ----- inherited from AbstractSource -----

            virtual TypeDescr source_ev_type() const override {
                return reflect::Reflect::require<Event>();
            } /*source_ev_type*/

            virtual uint32_t n_out_ev() const override { return n_out_ev_; }
            /* #of events queued for delivery */
            virtual uint32_t n_queued_out_ev() const override { return this->reducer_.n_event(); }

            virtual bool debug_sim_flag() const override { return debug_sim_flag_; }
            virtual void set_debug_sim_flag(bool x) override { this->debug_sim_flag_ = x; }

            virtual CallbackId attach_sink(rp<AbstractSink> const & sink) override {
                rp<EventSink> native_sink
                    = EventSink::require_native("SecondarySource::attach_sink", sink);

                if (native_sink) {
                    if (!this->is_volatile()
                        || native_sink->allow_volatile_source())
                    {
                        return this->add_callback(native_sink);
                    } else {
                        throw std::runtime_error("SecondarySource::attach_sink"
                                                 ": sink requires non-volatile source "
                                                 + std::string(reflect::type_name<Event>()));
                    }
                } else {
                    throw std::runtime_error("SecondarySource::attach_sink"
                                             ": expected sink accepting "
                                             + std::string(reflect::type_name<Event>()));
                }
            } /*attach_sink*/

            virtual void detach_sink(CallbackId id) override {
                this->remove_callback(id);
            } /*detach_sink*/

            // ----- Inherited from AbstractEventProcessor -----

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { this->name_ = x; }

            virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor> ep)> const & fn) override {

                for(auto x : this->cb_set_)
                    fn(x.fn_.borrow());
            } /*visit_direct_consumers*/

        private:
            /* event book-keeping on receiving an event.
             */
            bool preprocess_secondary_event(Event const & ev)
                {
                    if (this->upstream_exhausted_) {
                        throw std::runtime_error("SecondarySource::notify_secondary_event"
                                                 ": not allowed after upstream exhausted");
                    }

                    utc_nanos evtm = this->reducer_.event_tm(ev);

                    if (this->current_tm_ < evtm)
                        this->current_tm_ = evtm;

                    /* if reducer is empty when .notify_event() begins,
                     * then reactor/simulator needs to be notified that source is no longer empty
                     */
                    bool is_priming = this->reducer_.is_empty();

                    return is_priming;
                } /*preprocess_secondary_event*/

            /* event bookkeeping after receiving an event.
             *
             * Require: event has been propagated to .reducer
             *
             * is_priming.  true if event causes source to
             *              become non-empty --> must notify reactor
             */
            void postprocess_secondary_event(bool is_priming) {

                Reactor * reactor = this->parent_reactor_;

                xo::pp::scope log(XO_DEBUG_(this->debug_sim_flag_),
                          xo::pp::xtag("name", name_),
                          xo::pp::xtag("reactor", (void*)reactor),
                          xo::pp::xtag("is_priming", is_priming));

                if (reactor) {
                    if (is_priming) {
                        /* reactor/simulator takes responsibility for delivering events */
                        reactor->notify_source_primed(bp<ReactorSource>::from_native(this));
                    }
                } else {
                    /* if no reactor,  deliver immediately */
                    this->deliver_one();
                }
            } /*postprocess_secondary_event*/

            /* deliver one event from reducer;
             * invoke callback whenever replay_flag is true
             */
            std::uint64_t deliver_one_aux(bool replay_flag) {
                xo::pp::scope log(XO_DEBUG_(this->debug_sim_flag_),
                          xo::pp::xtag("name", this->name_),
                          xo::pp::xtag("reducer.empty", this->reducer_.is_empty()),
                          xo::pp::xtag("replay_flag", replay_flag));

                if (this->reducer_.is_empty())
                    return 0;

                /* need to remove event _before_ invoking callbacks;
                 * callbacks may indirectly call this->notify_secondary_event(),
                 * modifiying .reducer
                 *
                 * reducer may use double-buffering scheme or similar to
                 * mitigate copying,  esp when Event objects are heavy
                 */
                Event & ev = this->reducer_.annex_one();

                /* if SecondarySource:
                 *   Event ev = this->event_heap_.front();
                 *   std::pop_heap(this->event_heap_.begin(),
                 *                 this->event_heap_.end(),
                 *                 std::greater<Event>());
                 *   this->event_heap_.pop_back();
                 */

                if (replay_flag) {
                    ++(this->n_out_ev_);
                    this->cb_set_.invoke(&EventSink::notify_ev, ev);
                }

                return 1;
            } /*deliver_one_aux*/

        private:
            /* current time for this source */
            utc_nanos current_tm_;

            /* reporting name for this source (use when .debug_sim_flag set)
             */
            std::string name_;

            /* if true,  reactor/simulator to log interaction with this source
             */
            bool debug_sim_flag_ = false;

            /* count lifetime #of outgoing events */
            uint32_t n_out_ev_ = 0;

            /* set this to true,  once,  to announce that upstream will send
             * no more events.  see .notify_upstream_exhausted()
             */
            bool upstream_exhausted_ = false;

            /* events to be delivered to callbacks.
             * multiple events may be collapsed depending on Reducer implementation
             */
            Reducer reducer_;

            /* reactor/simulator being used to schedule consumption.  if ommitted,
             * will borrow thread calling .notify_secondary_event()
             */
            Reactor * parent_reactor_ = nullptr;

            /* invoke callbacks in this set to send an outgoing event */
            RpCallbackSet<EventSink> cb_set_;
        }; /*SecondarySource*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end SecondarySource.hpp */
