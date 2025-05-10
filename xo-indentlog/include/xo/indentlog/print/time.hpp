/* @file time.hpp */

#pragma once

#include "xo/indentlog/timeutil/timeutil.hpp"

namespace xo {
    namespace time {
        // ----- iso8601 -----

        /* stream inserter that displays time in ISO 8601 format:
         *   2012-04-23T18:25:43.511Z
         */
        struct iso8601 {
            iso8601(utc_nanos t0) : t0_{t0} {}

            utc_nanos t0_;
        }; /*iso8601*/

        inline std::ostream &
        operator<<(std::ostream & os,
                   iso8601 x)
        {
            timeutil::print_iso8601(x.t0_, os);
            return os;
        } /*operator<<*/

        // ----- hms_msec -----

        /* stream inserter that display time like:
         *   hh:mm:ss.nnn
         */
        struct hms_msec {
            hms_msec(nanos dt) : dt_{dt} {}

            static hms_msec utc(utc_nanos t0) { return hms_msec(timeutil::utc_split_vs_midnight(t0).second); }
            static hms_msec local(utc_nanos t0) { return hms_msec(timeutil::local_split_vs_midnight(t0).second); }

            nanos dt_;
        }; /*hms_msec*/

        inline std::ostream &
        operator<<(std::ostream & os, hms_msec x)
        {
            timeutil::print_hms_msec(x.dt_, os);
            return os;
        } /*operator<<*/

        // ----- hms_usec -----

        /* stream inserter that display time like:
         *   hh:mm:ss.nnnnnn
         */
        struct hms_usec {
            hms_usec(nanos dt) : dt_{dt} {}

            static hms_usec utc(utc_nanos t0) { return hms_usec(timeutil::utc_split_vs_midnight(t0).second); }
            static hms_usec local(utc_nanos t0) { return hms_usec(timeutil::local_split_vs_midnight(t0).second); }

            nanos dt_;
        }; /*hms_msec*/

        inline std::ostream &
        operator<<(std::ostream & os, hms_usec x)
        {
            timeutil::print_hms_usec(x.dt_, os);
            return os;
        } /*operator<<*/
    } /*namespace time*/
} /*namespace xo*/

namespace std {
    namespace chrono {
        inline std::ostream & operator<<(std::ostream & os,
                                         xo::time::utc_nanos t0)
        {
            xo::time::timeutil::print_utc_ymd_hms_usec(t0, os);
            return os;
        } /*operator<<*/

        inline std::ostream & operator<<(std::ostream & os,
                                         xo::time::nanos dt)
        {
            xo::time::timeutil::print_hms_usec(dt, os);
            return os;
        } /*operator<<*/
    } /*namespace chrono*/
} /*namespace std*/

/* end time.hpp */
