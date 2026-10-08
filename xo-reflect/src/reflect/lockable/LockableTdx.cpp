/** @file LockableTdx.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "lockable/LockableTdx.hpp"
#include "TaggedPtr.hpp"
#include "TypeDescr.hpp"

namespace xo {
    namespace reflect {
        std::unique_ptr<LockableTdx>
        LockableTdx::make(LockFn read_lock, LockFn read_unlock, TryLockFn try_read_lock)
        {
            return std::unique_ptr<LockableTdx>(new LockableTdx(read_lock, read_unlock,
                                                                try_read_lock));
        } /*make*/

        TaggedPtr
        LockableTdx::child_tp(uint32_t /*i*/, void * /*object*/) const {
            return TaggedPtr::universal_null();
        } /*child_tp*/

        TypeDescr
        LockableTdx::fixed_child_td(uint32_t /*i*/) const {
            return nullptr;
        } /*fixed_child_td*/

        std::string const &
        LockableTdx::struct_member_name(uint32_t i) const {
            return TypeDescrExtra::struct_member_name(i);
        } /*struct_member_name*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end LockableTdx.cpp */
