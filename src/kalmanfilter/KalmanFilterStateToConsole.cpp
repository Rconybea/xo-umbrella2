/* @file KalmanFilterStateToConsole.cpp */

#include "KalmanFilterStateToConsole.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
  using xo::xtag;

  namespace kalman {
    ref::rp<KalmanFilterStateToConsole>
    KalmanFilterStateToConsole::make() {
      return new KalmanFilterStateToConsole();
    } /*make*/

    void
    KalmanFilterStateToConsole::display(std::ostream & os) const
    {
      os << "<KalmanFilterStateToConsole"
     << xtag("this", (void*)this)
     << ">";
    } /*display*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterStateToConsole.cpp */
