/** @file WrapperTdx.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "wrapper/WrapperTdx.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescr.hpp"

namespace xo {
    namespace reflect {
        std::unique_ptr<WrapperTdx>
        WrapperTdx::make(std::unique_ptr<AbstractStructMemberAccessor> accessor)
        {
            return std::unique_ptr<WrapperTdx>(new WrapperTdx(std::move(accessor)));
        } /*make*/

        TaggedPtr
        WrapperTdx::wrapped_tp(void * object) const
        {
            return accessor_->member_tp(object);
        } /*wrapped_tp*/

        TaggedPtr
        WrapperTdx::child_tp(uint32_t /*i*/, void * /*object*/) const {
            return TaggedPtr::universal_null();
        } /*child_tp*/

        TypeDescr
        WrapperTdx::fixed_child_td(uint32_t /*i*/) const {
            return nullptr;
        } /*fixed_child_td*/

        std::string const &
        WrapperTdx::struct_member_name(uint32_t i) const {
            return TypeDescrExtra::struct_member_name(i);
        } /*struct_member_name*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end WrapperTdx.cpp */
