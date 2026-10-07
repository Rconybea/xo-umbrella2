/** @file StdAtomicTdx.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "atomic/StdAtomicTdx.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescr.hpp"

namespace xo {
    namespace reflect {
        std::unique_ptr<StdAtomicTdx>
        StdAtomicTdx::make(const TypeDescrBase * value_td,
                           std::size_t value_size,
                           std::size_t value_align,
                           LoadFn load)
        {
            return std::unique_ptr<StdAtomicTdx>(new StdAtomicTdx(value_td, value_size,
                                                                  value_align, load));
        } /*make*/

        TaggedPtr
        StdAtomicTdx::child_tp(uint32_t /*i*/, void * /*object*/) const {
            return TaggedPtr::universal_null();
        } /*child_tp*/

        TypeDescr
        StdAtomicTdx::fixed_child_td(uint32_t /*i*/) const {
            return nullptr;
        } /*fixed_child_td*/

        std::string const &
        StdAtomicTdx::struct_member_name(uint32_t i) const {
            return TypeDescrExtra::struct_member_name(i);
        } /*struct_member_name*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end StdAtomicTdx.cpp */
