/* @file KalmanFilterStateToConsole.hpp */

#pragma once

#include "xo/reactor/Sink.hpp"
#include "KalmanFilterState.hpp"

namespace xo {
    namespace kalman {
        class KalmanFilterStateToConsole
            : public xo::reactor::SinkToConsole<KalmanFilterStateExt>
        {
        public:
            KalmanFilterStateToConsole() = default;

            static rp<KalmanFilterStateToConsole> make();

            virtual void display(std::ostream & os) const;
            //virtual std::string display_string() const;
        }; /*KalmanFilterStateToConsole*/

        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterStateToConsole const & x) {
            x.display(os);
            return os;
        } /*operator<<*/
    } /*namespace option*/
} /*namespace xo*/

/* end KalmanFilterStateToConsole.hpp */
