/* @file Reducer.hpp */

#pragma once

#include "xo/reactor/EventTimeFn.hpp"

namespace xo {
    namespace reactor {
        /* LastReducer, HeapReducer inherit ReducerBase */
        template<typename Event, typename EventTimeFn>
        class ReducerBase {
            static_assert(EventTimeConcept<Event, EventTimeFn>);

        public:
            using utc_nanos = xo::time::utc_nanos;

        public:
            ReducerBase() = default;
            ReducerBase(EventTimeFn const & evtfn) : event_tm_fn_{evtfn} {}

            utc_nanos event_tm(Event const & ev) const { return this->event_tm_fn_(ev); }

        private:
            /*   Event ev = ...;
             *   .event_tm_fn(ev) -> utc_nanos
             * reports event time associated with ev
             */
            EventTimeFn event_tm_fn_;
        }; /*ReducerBase*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end Reducer.hpp */
