/* file PointerTdx.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "pointer/PointerTdx.hpp"

namespace xo {
  namespace reflect {
    std::string const &
    PointerTdx::struct_member_name(uint32_t i) const {
      return TypeDescrExtra::struct_member_name(i);
    } /*struct_member_name*/
  } /*namespace reflect*/
} /*namespace xo*/

/* end PointerTdx.cpp */
