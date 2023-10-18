/* @file UpxEvent.hpp */

#pragma once

#include "xo/indentlog/timeutil/timeutil.hpp"

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
            //UpxEvent(std::pair<utc_nanos, double> const & x) : contents_{x} {}
            UpxEvent(std::pair<utc_nanos, double> const & x) : tm_{x.first}, upx_{x.second} {}
            //UpxEvent(utc_nanos tm, double x) : contents_{tm, x} {}
            UpxEvent(utc_nanos tm, double x) : tm_{tm}, upx_{x} {}

            /* reflect UpxEvent object representation */
            static void reflect_self();

            /* convenience -- e.g. so we can use with EventTimeFn */
            //utc_nanos tm() const { return contents_.first; }
            utc_nanos tm() const { return tm_; }
            //double upx() const { return contents_.second; }
            double upx() const { return upx_; }

            void display(std::ostream & os) const;
            std::string display_string() const;

        private:
            /* note: earlier version inherited std::pair<>,  but this exposed
             *       pybind11 problem when we tried to control printing
             */
            utc_nanos tm_;
            double upx_;
            //std::pair<utc_nanos, double> contents_;
        }; /*UpxEvent*/

        inline std::ostream &
        operator<<(std::ostream & os, UpxEvent const & x) {
            x.display(os);
            return os;
        } /*operator<<*/

    } /*namespace process*/
} /*namespace xo*/

/* end UpxEvent.hpp */
