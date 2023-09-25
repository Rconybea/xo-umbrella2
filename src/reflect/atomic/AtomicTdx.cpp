/* @file AtomicTdx.cpp */

#include "atomic/AtomicTdx.hpp"
#include "TaggedPtr.hpp"

namespace xo {
  namespace reflect {
    std::unique_ptr<AtomicTdx> AtomicTdx::make() {
      return std::unique_ptr<AtomicTdx>(new AtomicTdx());
    } /*make*/

     TaggedPtr
     AtomicTdx::child_tp(uint32_t /*i*/, void * /*object*/) const {
       return TaggedPtr::universal_null();
     } /*child_tp*/

    std::string const &
    AtomicTdx::struct_member_name(uint32_t i) const {
      return TypeDescrExtra::struct_member_name(i);
    } /*struct_member_name*/
  } /*namespace reflect*/
} /*namespace xo*/

/* end AtomicTdx.cpp */
