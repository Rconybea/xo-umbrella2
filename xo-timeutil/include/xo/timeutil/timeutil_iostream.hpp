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

namespace xo {
    namespace time {
        inline void print_hms_msec(nanos dt, std::ostream & os) {
            /* use hhmmss.nnn -- format lives in timeutil.hpp::format_hms_msec */
            char buf[16];
            os << format_hms_msec(buf, sizeof(buf), dt);
        } /*print_hms_msec*/

        inline void print_utc_hms_msec(utc_nanos t0, std::ostream & os) {
            char buf[16];
            os << format_utc_hms_msec(buf, sizeof(buf), t0);
        } /*print_utc_hms_msec*/

        inline void print_hms_usec(nanos dt, std::ostream & os) {
            /* use hhmmss.uuuuuu -- format lives in timeutil.hpp::format_hms_usec */
            char buf[20];
            os << format_hms_usec(buf, sizeof(buf), dt);
        } /*print_hms_usec*/

        /* print t0 like:
         *   yyyymmdd:hh:mm:ss.uuuuuu
         * e.g.
         *   19700101:00:00:00.000000       // epoch
         *   20230921:16:29:35.123456       // 21sep2023 4:29:35 pm + 123456 us
         */
        inline void print_utc_ymd_hms_usec(utc_nanos t0, std::ostream & os) {
            char buf[32];
            os << format_utc_ymd_hms_usec(buf, sizeof(buf), t0);
        } /*print_utc_ymd_hms_usec*/

        /* print datetime in format compatible with ISO 8601.
         * copying the format javascript uses, e.g:
         *   2012-04-23T18:25:43.511Z
         */
        inline void print_iso8601(utc_nanos t0, std::ostream & os) {
            char buf[32];
            os << format_iso8601(buf, sizeof(buf), t0);
        } /*print_iso8601*/

    } /*namespace time*/
} /*namespace xo*/

/* end timeutil_iostream.hpp */
