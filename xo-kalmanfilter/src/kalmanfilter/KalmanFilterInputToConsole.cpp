/* @file KalmanFilterInputToConsole.cpp */

#include "KalmanFilterInputToConsole.hpp"
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */

namespace xo {

  namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
      using xo::pp::xtag;

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
