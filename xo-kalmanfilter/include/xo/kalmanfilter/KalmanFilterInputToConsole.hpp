/* @file KalmanFilterInputToConsole.hpp */

#pragma once

#include "KalmanFilterInput.hpp"
#include <xo/reactor/Sink.hpp>
#include <xo/ppsink/Prettifier.hpp>   /* pretty(PpSink&), Prettifier<> */

namespace xo {
    namespace kalman {
        class KalmanFilterInputToConsole
            : public xo::reactor::SinkToConsole<rp<KalmanFilterInput>>
        {
        public:
            KalmanFilterInputToConsole() = default;

            static rp<KalmanFilterInputToConsole> make();

            virtual void pretty(xo::pp::PpSink & sink) const;
#ifdef OBSOLETE
            virtual void display(std::ostream & os) const;
#endif
            //virtual std::string display_string() const;
        }; /*KalmanFilterInputToConsole*/

#ifdef OBSOLETE
        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterInputToConsole const & x) {
            x.display(os);
            return os;
        } /*operator<<*/
#endif
    } /*namespace option*/

    namespace pp {
        template <>
        struct Prettifier<xo::kalman::KalmanFilterInputToConsole> {
            static void print(PpSink & sink, const xo::kalman::KalmanFilterInputToConsole & x) {
                x.pretty(sink);
            }
        };
    }
} /*namespace xo*/

/* end KalmanFilterInputToConsole.hpp */
