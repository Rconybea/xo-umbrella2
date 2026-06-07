/* @file EventTimeFn2.hpp */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include <timeutil/timeutil.hpp>

namespace xo {
    namespace reactor {
        template <typename Event>
        class EventTimeFn {
        public:
            using utc_nanos = xo::time::utc_nanos;
            using event_t = Event;

        public:
            static utc_nanos event_tm(event_t const & ev) { return ev.tm(); }

            utc_nanos operator()(event_t const & ev) const { return EventTimeFn::event_tm(ev); }
        };

        template <typename T>
        class EventTimeFn<xo::rp<T>> {
        public:
            using utc_nanos = xo::time::utc_nanos;
            using event_t = xo::rp<T>;

        public:
            static utc_nanos event_tm(event_t const & ev) { return ev->tm(); }

            utc_nanos operator()(event_t const & ev) const { return EventTimeFn::event_tm(ev); }
        };

        template <typename T>
        class EventTimeFn<T*> {
        public:
            using utc_nanos = xo::time::utc_nanos;
            using event_t = T*;

        public:
            static utc_nanos event_tm(event_t ev) { return ev->tm(); }

            utc_nanos operator()(event_t const & ev) const { return EventTimeFn::event_tm(ev); }
        };

        template <typename T>
        class EventTimeFn<std::pair<xo::time::utc_nanos, T>> {
        public:
            using utc_nanos = xo::time::utc_nanos;
            using event_t = std::pair<xo::time::utc_nanos, T>;

        public:
            static utc_nanos event_tm(event_t const & ev) { return ev.first; }

            utc_nanos operator()(event_t const & ev) const { return EventTimeFn::event_tm(ev); }
        };
    } /*namespace reactor*/
} /*namespace xo*/

/* end EventTimeFn2.hpp */
