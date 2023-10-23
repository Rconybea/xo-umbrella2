/* @file KalmanFilterSpec.cpp */

#include "KalmanFilterSpec.hpp"
#include "indentlog/scope.hpp"

namespace xo {
  using xo::tostr;
  using xo::xtag;

  namespace kalman {
    void
    KalmanFilterSpec::display(std::ostream & os) const
    {
      os << "<KalmanFilterSpec"
     << xtag("start_ext", start_ext_)
     << ">";
    } /*display*/

    std::string
    KalmanFilterSpec::display_string() const
    {
      return tostr(*this);
    } /*display_string*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSpec.cpp */
