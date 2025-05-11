/* @file Sink.cpp */

#include "Sink.hpp"
#include "xo/refcnt/Refcounted.hpp"

namespace xo {
  namespace reactor {
#ifdef NOT_USING
    rp<SinkToConsole<std::pair<xo::time::utc_nanos, double>>>
    TemporaryTest::realization_printer()
    {
      return new SinkToConsole<std::pair<xo::time::utc_nanos, double>>();
    } /*realization_printer*/
#endif
  } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.cpp */
