/* @file KalmanFilterInputToConsole.cpp */

#include "KalmanFilterInputToConsole.hpp"
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

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

#ifdef OBSOLETE
    void
    KalmanFilterInputToConsole::display(std::ostream & os) const
    {
      os << "<KalmanFilterInputToConsole"
     << xtag("this", (void*)this)
     << ">";
    } /*display*/
#endif

        void
        KalmanFilterInputToConsole::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("KalmanFilterInputToConsole",
                               field("this", (void*)this));
        }
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputToConsole.cpp */
