/* @file FifoQueue.hpp */

#pragma once

#include "Reactor.hpp"
#include "EventSource.hpp"
#include "Sink.hpp"
#include "EventTimeFn2.hpp"
#include "xo/callback/CallbackSet.hpp"
#include <deque>

namespace xo {
    namespace reactor {
        /* require:
         *   T null constructible
         *   T movable
         *
         * T satisfies EventTimeConcept
         */
        template <typename T, typename EvTimeFn = EventTimeFn<T>>
        class FifoQueue : public virtual Sink1<T>, public virtual EventSource<Sink1<T>> {
        public:
            using EventSink = Sink1<T>;
            template<typename Fn>
            using RpCallbackSet = xo::fn::RpCallbackSet<Fn>;
            using CallbackId = xo::fn::CallbackId;
            using Reflect = xo::reflect::Reflect;
            using TypeDescr = xo::reflect::TypeDescr;
            using utc_nanos = xo::time::utc_nanos;

        public:
            static rp<FifoQueue> make(EvTimeFn evtm_fn = EvTimeFn()) { return new FifoQueue(evtm_fn); }

            // ----- inherited from Sink1<T> -----

            virtual void notify_ev(T const & ev) override {
                bool is_priming = this->elt_q_.empty();

                this->elt_q_.push_back(ev);

                ++(this->n_in_ev_);

                if (this->upstream_exhausted_) {
                    throw std::runtime_error("FifoQueue::notify_ev"
                                             ": not allowed after upstream exhausted");
                }

                utc_nanos tm = evtm_fn_(ev);

                if (this->current_tm_ < tm)
                    this->current_tm_ = tm;

                Reactor * reactor = this->parent_reactor_;

                scope log(XO_DEBUG(this->debug_sim_flag_),
                          xtag("name", name_),
                          xtag("reactor", (void*)reactor),
                          xtag("is_priming", is_priming));

                if (reactor) {
                    if (is_priming) {
                        /* reactor/simulator takes delivery/sequencing responsibility from here */
                        reactor->notify_source_primed(bp<ReactorSource>::from_native(this));
                    }
                } else {
                    /* if no reactor,  deliver immediately */
                    this->deliver_one();
                }
            } /*notify_ev*/

            // ----- inherited from AbstractSink -----

            /* we don't care about volatile sources -- fifo queue copies incoming events */
            virtual bool allow_volatile_source() const override { return true; }

            virtual uint32_t n_in_ev() const override { return n_in_ev_; }

            // ----- inherited from ReactorSource -----

            virtual bool is_empty() const override { return elt_q_.empty(); }
            virtual bool is_exhausted() const override { return this->upstream_exhausted_ && this->is_empty(); }

            virtual utc_nanos sim_current_tm() const override {
                if (this->elt_q_.empty()) {
                    /* (in practice control never comes here)
                     *
                     * queue doesn't know time of next event yet;
                     * new events may appear at any time by way of .notify_event()
                     *
                     * if queue doesn't know next event,  can't use .sim_current_tm
                     * to establish priority relative to other sources.
                     * In that case rely instead on priming mechanism;
                     * priming mechanism implies control should never come here
                     */
                    return this->current_tm_;
                } else {
                    return evtm_fn_(this->elt_q_.front());
                }
            } /*sim_current_tm*/

            virtual uint64_t deliver_one() override {
                return this->deliver_one_aux(true /*replay_flag*/);
            } /*deliver_one*/

            virtual uint64_t sim_advance_until(utc_nanos target_tm,
                                               bool replay_flag) override {
                uint64_t retval = 0;

                while (!this->elt_q_.empty()) {
                    utc_nanos tm = evtm_fn_(this->elt_q_.front());

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

            virtual void notify_reactor_remove(Reactor *) override {
                this->parent_reactor_ = nullptr;
            }

            // ----- inherited from AbstractSource -----

            virtual TypeDescr source_ev_type() const override { return Reflect::require<T>(); }
            /* events must be copied objects owned by FifoQueue.
             * not expected to be pointers to shared storage or something
             */
            virtual bool is_volatile() const override { return false; }
            virtual uint32_t n_queued_out_ev() const override { return elt_q_.size(); }
            virtual uint32_t n_out_ev() const override { return n_out_ev_; }
            virtual bool debug_sim_flag() const override { return debug_sim_flag_; }
            virtual void set_debug_sim_flag(bool x) override { this->debug_sim_flag_ = x; }

            virtual CallbackId attach_sink(rp<AbstractSink> const & sink) override {
                rp<EventSink> native_sink
                    = EventSink::require_native("FifoQueue::attach_sink", sink);

                if (native_sink) {
                    if (!this->is_volatile()
                        || native_sink->allow_volatile_source())
                    {
                        return this->add_callback(native_sink);
                    } else {
                        throw std::runtime_error("FifoQueue::attach_sink"
                                                 ": sink requires non-volatile source "
                                                 + std::string(reflect::type_name<T>()));
                    }
                } else {
                    throw std::runtime_error("FifoQueue::attach_sink"
                                             ": expected sink accepting "
                                             + std::string(reflect::type_name<T>()));
                }
            } /*attach_sink*/

            virtual void detach_sink(CallbackId id) override {
                this->remove_callback(id);
            }

            // ----- inherited from EventSource -----

            virtual CallbackId add_callback(rp<EventSink> const & cb) override {
                return this->cb_set_.add_callback(cb);
            }

            virtual void remove_callback(CallbackId id) override {
                this->cb_set_.remove_callback(id);
            }

            // ----- inherited from AbstractEventProcessor -----

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { this->name_ = x; }

            virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor> ep)> const & fn) override {
                for (auto x : this->cb_set_)
                    fn(x.fn_.borrow());
            } /*visit_direct_consumers*/

            /* write human-readable representation to stream */
            virtual void display(std::ostream & os) const override {
                os << "<FifoQueue"
                   << xtag("name", name_)
                   << xtag("addr", (void *)this)
                   << xtag("T", reflect::type_name<T>())
                   << ">";
            } /*display*/

        private:
            FifoQueue(EvTimeFn evtm_fn) : evtm_fn_{std::move(evtm_fn)} {}

            uint64_t deliver_one_aux(bool replay_flag) {
                scope log(XO_DEBUG(this->debug_sim_flag_),
                          xtag("name", this->name_),
                          xtag("elt_q.size", this->elt_q_.size()),
                          xtag("replay_flag", replay_flag));

                if (this->elt_q_.empty())
                    return 0;

                /* avoiding copy for efficiently-swappable T */
                T ev;
                std::swap(ev, this->elt_q_.front());

                this->elt_q_.pop_front();

                if (replay_flag) {
                    log && log(xtag("deliver-ev", ev),
                               xtag("elt_q.size", this->elt_q_.size()));

                    ++(this->n_out_ev_);
                    this->cb_set_.invoke(&EventSink::notify_ev, ev);
                }

                return 1;
            } /*deliver_one_aux*/

        private:
            /* name (ideally unique) for this queue */
            std::string name_;

            /* extract timestamp from an event */
            EvTimeFn evtm_fn_;

            /* if true, simulator/reactor will report interaction with this source */
            bool debug_sim_flag_ = false;

            /* largest event timestamp delivered
             * (monotonically increases,  event if events received out-of-timestamp-order)
             */
            utc_nanos current_tm_;

            /* events waiting for delivery */
            std::deque<T> elt_q_;

            /* lifetime #of events received */
            uint32_t n_in_ev_ = 0;
            /* lifetime #of events delivered */
            uint32_t n_out_ev_ = 0;

            /* set to true, once, to announce that upstream will send no more events.
             * see .notify_upstream_exhausted() ?
             */
            bool upstream_exhausted_ = false;

            /* reactor/simulator being used to schedule event consumption.
             * if omitted,  borrow calling thread
             */
            Reactor * parent_reactor_ = nullptr;

            /* invoke callbacks in this set to deliver queued events */
            RpCallbackSet<EventSink> cb_set_;

        }; /*FifoQueue*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end FifoQueue.hpp */
