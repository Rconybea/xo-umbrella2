/* file StructMember.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "struct/StructMember.hpp"
#include "indentlog/scope.hpp"
#include <type_traits>

namespace xo {
  using xo::scope;
  using xo::xtag;

  namespace reflect {
    static_assert(std::is_move_constructible_v<StructMember>);

    TaggedPtr
    AbstractStructMemberAccessor::member_tp(void * struct_addr) const
    {
      //XO_SCOPE(lscope);

      TaggedPtr retval = (this
              ->member_td()
              ->most_derived_self_tp(this->address(struct_addr)));

      //lscope.log(xtag("self_td", this->struct_td()->short_name()),
      //           xtag("member_td.declared", this->member_td()->short_name()),
      //           xtag("member_td.actual", retval.td()->short_name()));

      return retval;
    } /*member_tp*/

  } /*namespace reflect*/
} /*namespace xo*/

/* end StructMember.cpp */
