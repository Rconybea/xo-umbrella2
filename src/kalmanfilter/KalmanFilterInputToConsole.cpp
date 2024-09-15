/* @file KalmanFilterInputToConsole.cpp */

#include "KalmanFilterInputToConsole.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
  using xo::xtag;

  namespace kalman {
    rp<KalmanFilterInputToConsole>
    KalmanFilterInputToConsole::make() {
      return new KalmanFilterInputToConsole();
    } /*make*/

    void
    KalmanFilterInputToConsole::display(std::ostream & os) const
    {
      os << "<KalmanFilterInputToConsole"
     << xtag("this", (void*)this)
     << ">";
    } /*display*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputToConsole.cpp */
