/* file TypeDescrExtra.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "TypeDescrExtra.hpp"
#include "TypeDescr.hpp"
#include "TaggedPtr.hpp"
#include <cassert>

namespace xo {
  namespace reflect {
    TaggedPtr
    TypeDescrExtra::most_derived_self_tp(TypeDescrBase const * object_td,
					 void * object) const
    {
      return TaggedPtr(object_td, object);
    } /*most_derived_self_tp*/

    std::string const &
    TypeDescrExtra::struct_member_name(uint32_t /*i*/) const {
      assert(false);

      static std::string s_null;
      return s_null;
    } /*struct_member_name*/
    
    StructMember const *
    TypeDescrExtra::struct_member(uint32_t /*i*/) const {
      assert(false);

      return nullptr;
    } /*struct_member*/
  } /*namespace reflect*/
} /*namespace xo*/

/* end TypeDescrExtra.cpp */
