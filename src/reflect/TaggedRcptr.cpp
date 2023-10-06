/* file TaggedRcptr.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "TaggedRcptr.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
  using xo::xtag;
  using xo::tostr;

  namespace reflect {
    void
    TaggedRcptr::display(std::ostream & os) const
    {
      os << "<TaggedRcptr"
     << xtag("type", this->td()->canonical_name())
     << xtag("addr", this->rc_address())
     << ">";
    } /*display*/

    std::string
    TaggedRcptr::display_string() const {
      return tostr(*this);
    } /*display_string*/
  } /*namespace reflect*/
} /*namespace xo*/

/* end TaggedRcptr.cpp */
