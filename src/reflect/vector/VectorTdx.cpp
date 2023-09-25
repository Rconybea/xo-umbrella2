/* file VectorTdx.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "vector/VectorTdx.hpp"

namespace xo {
  namespace reflect {
    std::string const &
    VectorTdx::struct_member_name(uint32_t i) const {
      return TypeDescrExtra::struct_member_name(i);
    } /*struct_member_name*/

  } /*namespace reflect*/

} /*namespace xo*/


/* end VectorTdx.cpp */
