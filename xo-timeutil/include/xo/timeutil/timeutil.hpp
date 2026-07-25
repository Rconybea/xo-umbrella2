/** @file timeutil.hpp
 *
 *  Time vocabulary types + calendar math.
 *
 *  Deliberately has NO <ostream> dependency, so downstream code can use
 *  time types without pulling in iostream machinery.  For ostream
 *  formatting (print_iso8601 etc.) include timeutil_iostream.hpp instead.
 *
 *  @author Roland Conybeare
 **/

#pragma once

#include <chrono>
#include <compare>
#include <utility>
#include <cstdint>
#include <ctime>
#include <time.h>

namespace xo {
    namespace time {
        using nanos = std::chrono::nanoseconds;
        using microseconds = std::chrono::microseconds;
        using milliseconds = std::chrono::milliseconds;
        using seconds = std::chrono::seconds;
        using hours = std::chrono::hours;
        using days = std::chrono::days;

        using utc_nanos = std::chrono::time_point<std::chrono::system_clock,
                                                  std::chrono::nanoseconds>;
        using utc_micros = std::chrono::time_point<std::chrono::system_clock,
                                                   std::chrono::microseconds>;

        struct timeutil {
            static utc_nanos now() {
                return utc_nanos(std::chrono::system_clock::now());
            }

            static utc_nanos epoch() {
                return utc_nanos(std::chrono::system_clock::from_time_t(0));
            } /*epoch*/

            static utc_nanos ymd_hms(uint32_t ymd, uint32_t hms) {
                /* e.g. ymd=20220610 -> n_yr=2022, n_mon=06, n_dy=10 */

                uint32_t n_yr = ymd / 10000;
                uint32_t n_mon = (ymd % 10000) / 100;
                uint32_t n_dy = ymd % 100;

                uint32_t n_hr = hms / 10000;
                uint32_t n_min = (hms % 10000) / 100;
                uint32_t n_sec = hms % 100;

                struct tm t;

                t.tm_year = n_yr - 1900; /* 0 means 1900 */
                t.tm_mon = n_mon - 1;    /* 0 means january */
                t.tm_mday = n_dy;

                t.tm_hour = n_hr;    /* 24 hour clock */
                t.tm_min = n_min;
                t.tm_sec = n_sec;

                /* time since epoch */
                time_t epoch_time = timegm(&t);

                return std::chrono::system_clock::from_time_t(epoch_time);
            } /*ymd_hms*/

            /* midnight UTC on date ymd.
             *   e.g. ymd_midnight(20220707) -> midnight UTC on 7jul22
             */
            static utc_nanos ymd_midnight(uint32_t ymd) {
                return ymd_hms(ymd, 0);
            } /*ymd_midnight*/

            static utc_nanos ymd_hms_usec(uint32_t ymd, uint32_t hms, uint32_t usec) {
                utc_nanos s = ymd_hms(ymd, hms);

                return s + microseconds(usec);
            } /*ymd_hms_usec*/

            /* .first:  UTC midnight on same calendar day as t0
             * .second: elapsed time from .first to t0 (i.e. UTC time-of-day for t0)
             */
            static std::pair<utc_nanos, nanos> utc_split_vs_midnight(utc_nanos t0) {
                /* use yyyymmdd.hh:mm:ss.nnnnnn */

                time_t t0_time_t = (std::chrono::system_clock::to_time_t
                                    (std::chrono::time_point_cast<microseconds>(t0)));

                /* convert to std::tm,
                 * only provides 1-second precision
                 */
                std::tm t0_tm;
                ::gmtime_r(&t0_time_t, &t0_tm);

                /* midnight on the same calendar day as t0_tm */
                std::tm midnight_tm = t0_tm;
                {
                    midnight_tm.tm_hour = 0;
                    midnight_tm.tm_min = 0;
                    midnight_tm.tm_sec = 0;
                }

                /* convert to UTC epoch seconds */
                time_t midnight_time_t = ::timegm(&midnight_tm);

                utc_nanos t0_midnight =
                    (std::chrono::time_point_cast<microseconds>(
                        std::chrono::system_clock::from_time_t(midnight_time_t)));

                nanos t0_tdy = t0 - t0_midnight;

                return std::pair<utc_nanos, nanos>(t0_midnight, t0_tdy);
            } /*utc_split_vs_midnight*/

            /* .first:  LOCAL midnight on same calendar day as t0 (but in UTC coords)
             * .second: elapsed time from .first to t0 (i.e. LOCAL time-of-day for t0)
             */
            static std::pair<utc_nanos, nanos> local_split_vs_midnight(utc_nanos t0) {
                /* use yyyymmdd.hh:mm:ss.nnnnnn */

                time_t t0_time_t = (std::chrono::system_clock::to_time_t
                                    (std::chrono::time_point_cast<microseconds>(t0)));

                /* convert to std::tm,
                 * only provides 1-second precision
                 */
                std::tm t0_tm;
                ::localtime_r(&t0_time_t, &t0_tm);

                /* midnight on the same calendar day as t0_tm */
                std::tm midnight_tm = t0_tm;
                {
                    midnight_tm.tm_hour = 0;
                    midnight_tm.tm_min = 0;
                    midnight_tm.tm_sec = 0;
                }

                /* convert local midnight to UTC epoch seconds */
                time_t midnight_time_t = ::timelocal(&midnight_tm);

                utc_nanos t0_midnight =
                    (std::chrono::time_point_cast<microseconds>(
                        std::chrono::system_clock::from_time_t(midnight_time_t)));

                nanos t0_tdy = t0 - t0_midnight;

                return std::pair<utc_nanos, nanos>(t0_midnight, t0_tdy);
            } /*local_split_vs_midnight*/

            /* split utc_nanos into
             *   std::tm
             *    .tm_year
             *    .tm_mon   (1-12)
             *    .tm_mday  (1-31)
             *    .tm_hour  (0-23)
             *    .tm_min   (0-59)
             *    .tm_sec   (0-59)
             *    .tm_wday  (0=sunday .. 6=saturday)
             *    .tm_yday  (0=1jan .. 365)
             *    .tm_isdst (daylight savings time flag)
             *   usec       (0-999999)
             */
            static std::pair<std::tm, uint32_t> utc_split_tm(utc_nanos t0) {
                /* use yyyymmdd.hh:mm:ss.nnnnnn */

                time_t t0_time_t
                    = (std::chrono::system_clock::to_time_t
                       (std::chrono::time_point_cast<microseconds>(t0)));

                /* convert to std::tm, in UTC coords,
                 * only provides 1-second precision
                 */
                std::tm t0_tm;
                ::gmtime_r(&t0_time_t, &t0_tm);

                /* midnight on the same calendar day as t0_tm */
                std::tm midnight_tm = t0_tm;

                midnight_tm.tm_isdst = 0;
                midnight_tm.tm_hour = 0;
                midnight_tm.tm_min = 0;
                midnight_tm.tm_sec = 0;

                /* convert back to epoch seconds */
                time_t midnight_time_t = ::timegm(&midnight_tm);

                utc_nanos t0_midnight =
                    (std::chrono::time_point_cast<microseconds>(
                        std::chrono::system_clock::from_time_t(midnight_time_t)));

                uint32_t usec =
                    (std::chrono::duration_cast<microseconds>(
                        std::chrono::hh_mm_ss(t0 - t0_midnight).subseconds()))
                    .count();

                return std::make_pair(t0_tm, usec);
            } /*utc_split_tm*/
        }; /*timeutil*/

    } /*namespace time*/
} /*namespace xo*/

/* end timeutil.hpp */
