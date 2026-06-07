/* @file AtomicTdx.cpp */

#include "atomic/AtomicTdx.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescr.hpp"
#include <sys/types.h>

namespace xo {
    namespace reflect {
        std::unique_ptr<AtomicTdx>
        AtomicTdx::make() {
            return std::unique_ptr<AtomicTdx>(new AtomicTdx());
        } /*make*/

        TaggedPtr
        AtomicTdx::child_tp(uint32_t /*i*/, void * /*object*/) const {
            return TaggedPtr::universal_null();
        } /*child_tp*/

        TypeDescr
        AtomicTdx::fixed_child_td(uint32_t /*i*/) const {
            return nullptr;
        }

        std::string const &
        AtomicTdx::struct_member_name(uint32_t i) const {
            return TypeDescrExtra::struct_member_name(i);
        } /*struct_member_name*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end AtomicTdx.cpp */
