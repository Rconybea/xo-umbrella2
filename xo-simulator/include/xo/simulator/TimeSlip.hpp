/* file TimeSlip.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

//#include "time/Time.hpp"

namespace xo {
    namespace sim {
        /* helper class for a throttled simulation,
         * where we want simulated time to evolve at a constant rate,
         * relative to real elapsed time.
         *
         * A TimeSlip instance pins
         * simulation-time coordinates to realtime coordinates
         */
        class TimeSlip {
        public:
            using utc_nanos = xo::time::utc_nanos;

        public:
            TimeSlip(utc_nanos sim_tm, utc_nanos real_tm)
                : sim_tm_{sim_tm}, real_tm_{real_tm} {}

            utc_nanos sim_tm() const { return sim_tm_; }
            utc_nanos real_tm() const { return real_tm_; }

        private:
            utc_nanos sim_tm_;
            utc_nanos real_tm_;
        }; /*TimeSlip*/
    } /*namespace sim*/

} /*namespace xo*/


/* end TimeSlip.hpp */
