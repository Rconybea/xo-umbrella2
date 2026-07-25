/** @file timeutil_iostream.hpp
 *
 *  ostream formatting for xo::time types.
 *
 *  Split out from timeutil.hpp so that timeutil.hpp itself carries no
 *  <ostream> dependency.  Include this header only where you actually
 *  format a time to a stream.
 *
 *  Note: these were previously static members timeutil::print_*; they are
 *  now free functions in namespace xo::time.
 *
 *  @author Roland Conybeare
 **/

#pragma once

#include "timeutil.hpp"
#include <ostream>
#include <iomanip>
#include <cstdio>

namespace xo {
    namespace time {
        inline void print_hms_msec(nanos dt, std::ostream & os) {
            /* use hhmmss.nnn */
            using std::int32_t;

            auto hms = std::chrono::hh_mm_ss(dt);
            int32_t h = hms.hours().count();
            int32_t m = hms.minutes().count();
            int32_t s = hms.seconds().count();
            int32_t msec = std::chrono::duration_cast<milliseconds>(hms.subseconds()).count();

            char buf[32];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d", h, m, s, msec);

            os << buf;
        } /*print_hms_msec*/

        inline void print_utc_hms_msec(utc_nanos t0, std::ostream & os) {
            print_hms_msec(timeutil::utc_split_vs_midnight(t0).second, os);
        } /*print_utc_hms_msec*/

        inline void print_hms_usec(nanos dt, std::ostream & os) {
            /* use hhmmss.uuuuuu */
            using std::int32_t;

            auto hms = std::chrono::hh_mm_ss(dt);
            int32_t h = hms.hours().count();
            int32_t m = hms.minutes().count();
            int32_t s = hms.seconds().count();
            int32_t usec = std::chrono::duration_cast<microseconds>(hms.subseconds()).count();

            char buf[32];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%06d", h, m, s, usec);

            os << buf;
        } /*print_hms_usec*/

        /* print t0 like:
         *   yyyymmdd:hh:mm:ss.uuuuuu
         * e.g.
         *   19700101:00:00:00.000000       // epoch
         *   20230921:16:29:35.123456       // 21sep2023 4:29:35 pm + 123456 us
         */
        inline void print_utc_ymd_hms_usec(utc_nanos t0, std::ostream & os) {
            /* (structured binding ftw!) */
            auto [t0_tm, t0_usec] = timeutil::utc_split_tm(t0);

            char usec_buf[15];
            snprintf(usec_buf, sizeof(usec_buf), "%06d", t0_usec);

            /* control string              | example
             * ----------------------------+--------------------------
             * %Y - year                   | 2022
             * %m - month                  | 06
             * %d - day of month           | 10
             * %H - hour                   | 16
             * %M - minute                 | 29
             * %S - second                 | 05
             */
            os << std::put_time(&t0_tm, "%Y%m%d:%H:%M:%S.") << usec_buf;
        } /*print_utc_ymd_hms_usec*/

        /* print datetime in format compatible with ISO 8601.
         * copying the format javascript uses, e.g:
         *   2012-04-23T18:25:43.511Z
         */
        inline void print_iso8601(utc_nanos t0, std::ostream & os) {
            auto [t0_tm, t0_usec] = timeutil::utc_split_tm(t0);

            char msec_buf[8];
            snprintf(msec_buf, sizeof(msec_buf), "%03d", t0_usec / 1000);

            os << std::put_time(&t0_tm, "%Y-%m-%dT%H:%M:%S.") << msec_buf << "Z";
        } /*print_iso8601*/

    } /*namespace time*/
} /*namespace xo*/

/* end timeutil_iostream.hpp */
