/* @file KalmanFilterStateToConsole.hpp */

#pragma once

#include "KalmanFilterState.hpp"
#include <xo/reactor/Sink.hpp>
#include <xo/ppsink/Prettifier.hpp>   /* pretty(PpSink&), Prettifier<> */

namespace xo {
    namespace kalman {
        class KalmanFilterStateToConsole
            : public xo::reactor::SinkToConsole<KalmanFilterStateExt>
        {
        public:
            KalmanFilterStateToConsole() = default;

            static rp<KalmanFilterStateToConsole> make();

            virtual void pretty(xo::pp::PpSink & sink) const;
#ifdef OBSOLETE
            virtual void display(std::ostream & os) const;
#endif
            //virtual std::string display_string() const;
        }; /*KalmanFilterStateToConsole*/

#ifdef OBSOLETE
        inline std::ostream &
        operator<<(std::ostream & os, KalmanFilterStateToConsole const & x) {
            x.display(os);
            return os;
        } /*operator<<*/
#endif
    } /*namespace option*/

    namespace pp {
        template <>
        struct Prettifier<xo::kalman::KalmanFilterStateToConsole> {
            static void print(PpSink & sink, const xo::kalman::KalmanFilterStateToConsole & x) {
                x.pretty(sink);
            }
        };
    }
} /*namespace xo*/

/* end KalmanFilterStateToConsole.hpp */
