/* @file HeapReducer.hpp */

#pragma once

#include "Reducer.hpp"

namespace xo {
    namespace reactor {
        /* collect incoming events in a heap,
         * ordered by timestamp.
         * output events in increasing timestamp order.
         * Information preserving in all other respects
         *
         * Require:
         * - Event is null-constructible
         * - Event is copyable
         * - EventTimeFn :: Event -> utc_nanos
         */
        template<typename Event, typename EventTimeFn = StructEventTimeFn<Event>>
        class HeapReducer : public ReducerBase<Event, EventTimeFn> {
        public:
            using utc_nanos = xo::time::utc_nanos;
        public:
            HeapReducer() = default;
            HeapReducer(EventTimeFn const & evtfn) : ReducerBase<Event, EventTimeFn>(evtfn) {}

            bool is_empty() const { return this->event_heap_.empty(); }
            /* require: .is_empty() = false */
            utc_nanos next_tm() const { return this->event_tm(this->event_heap_.front()); }
            /* #of events stored in this reducer */
            uint32_t n_event() const { return this->event_heap_.size(); }

            Event const & last_annexed_ev() const { return this->annexed_ev_; }

            void include_event(Event const & ev) {
                this->event_heap_.push_back(ev);
                std::push_heap(this->event_heap_.begin(),
                               this->event_heap_.end(),
                               std::greater<Event>());
            } /*include_event*/

            void include_event(Event && ev) {
                this->event_heap_.push_back(std::move(ev));
                std::push_heap(this->event_heap_.begin(),
                               this->event_heap_.end(),
                               std::greater<Event>());
            } /*include_event*/

            Event & annex_one() {
                this->annexed_ev_ = this->event_heap_.front();
                std::pop_heap(this->event_heap_.begin(),
                              this->event_heap_.end(),
                              std::greater<Event>());
                this->event_heap_.pop_back();

                return this->annexed_ev_;
            } /*annex_one*/

            // ----- Inherited from ReducerBase -----

            // utc_nanos event_tm(Event const & x);

        private:
            /* queued Events,  in increasing timestamp order */
            std::vector<Event> event_heap_;
            /* annexed event,  removed from .event_heap */
            Event annexed_ev_;
        }; /*HeapReducer*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end HeapReducer.hpp */
