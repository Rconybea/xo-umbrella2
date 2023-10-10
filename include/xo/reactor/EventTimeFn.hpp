/* @file EventTimeFn.hpp */

#pragma once

#include "time/Time.hpp"
#include <concepts>

namespace xo {
    namespace reactor {
        template <typename Event, typename EventTimeFn>
        concept EventTimeConcept = requires(EventTimeFn etfn, Event ev) {
            { etfn(ev) } -> std::same_as<xo::time::utc_nanos>;
        };

        template<typename Event>
        class StructEventTimeFn {
        public:
            using event_t = Event;
            using utc_nanos = xo::time::utc_nanos;
      
        public:
            utc_nanos operator()(Event const & ev) const { return ev.tm(); }
        }; /*StructEventTimeFn*/

        template<typename Event>
        class PtrEventTimeFn {
        public:
            using event_t =  Event;
            using utc_nanos = xo::time::utc_nanos;

        public:
            utc_nanos operator()(Event const & ev) const { return ev->tm(); }
        }; /*PtrEventTimeFn*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end EventTimeFn.hpp */
