/* @file KalmanFilterStateToConsole.cpp */

#include "KalmanFilterStateToConsole.hpp"
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {

  namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
      using xo::pp::xtag;

    rp<KalmanFilterStateToConsole>
    KalmanFilterStateToConsole::make() {
      return new KalmanFilterStateToConsole();
    } /*make*/

#ifdef OBSOLETE
    void
    KalmanFilterStateToConsole::display(std::ostream & os) const
    {
      os << "<KalmanFilterStateToConsole"
     << xtag("this", (void*)this)
     << ">";
    } /*display*/
#endif

        void
        KalmanFilterStateToConsole::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("KalmanFilterStateToConsole",
                               field("this", (void*)this));
        }
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterStateToConsole.cpp */
