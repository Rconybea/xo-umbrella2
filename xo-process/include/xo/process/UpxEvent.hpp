/* @file UpxEvent.hpp */

#pragma once

#include <xo/ppsink/Prettifier.hpp>   /* pretty(PpSink&), Prettifier<> */
#include <xo/timeutil/timeutil.hpp>

namespace xo {
    namespace process {
        /* typical representation for events emitted by a stochastic process
         * writing this as a non-template class (instead of just template alias)
         * because we want typeinfo to be generated
         */
        class UpxEvent {
        public:
            using utc_nanos = xo::time::utc_nanos;

        public:
            UpxEvent();
            UpxEvent(std::pair<utc_nanos, double> const & x) : tm_{x.first}, upx_{x.second} {}
            UpxEvent(utc_nanos tm, double x) : tm_{tm}, upx_{x} {}

            /* reflect UpxEvent object representation */
            static void reflect_self();

            /* convenience -- e.g. so we can use with EventTimeFn */
            utc_nanos tm() const { return tm_; }
            double upx() const { return upx_; }

            void pretty(xo::pp::PpSink & sink) const;
            std::string display_string() const;

        private:
            /* note: earlier version inherited std::pair<>,  but this exposed
             *       pybind11 problem when we tried to control printing
             */
            utc_nanos tm_;
            double upx_;
        }; /*UpxEvent*/

    } /*namespace process*/

    namespace pp {
        template <>
        struct Prettifier<xo::process::UpxEvent> {
            static void print(PpSink & sink, const xo::process::UpxEvent & x) {
                x.pretty(sink);
            }
        };
    }
} /*namespace xo*/

/* end UpxEvent.hpp */
