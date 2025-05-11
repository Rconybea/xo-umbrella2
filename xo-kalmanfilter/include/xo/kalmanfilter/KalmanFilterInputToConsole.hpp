/* @file KalmanFilterInputToConsole.hpp */

#pragma once

#include "xo/reactor/Sink.hpp"
#include "KalmanFilterInput.hpp"

namespace xo {
    namespace kalman {
        class KalmanFilterInputToConsole
            : public xo::reactor::SinkToConsole<rp<KalmanFilterInput>>
        {
        public:
            KalmanFilterInputToConsole() = default;

            static rp<KalmanFilterInputToConsole> make();

            virtual void display(std::ostream & os) const;
            //virtual std::string display_string() const;
        }; /*KalmanFilterInputToConsole*/

        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterInputToConsole const & x) {
            x.display(os);
            return os;
        } /*operator<<*/
    } /*namespace option*/
} /*namespace xo*/

/* end KalmanFilterInputToConsole.hpp */
