/** @file DAtomicType.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "AtomicType.hpp"

namespace xo {
    namespace scm {

        DAtomicType *
        DAtomicType::_make(obj<AAllocator> mm, Metatype mtype)
        {
            void * mem = mm.alloc_for<DAtomicType>();

            return new (mem) DAtomicType(mtype);
        }

        // ----- GCObject facet -----

        std::size_t
        DAtomicType::shallow_size() const noexcept
        {
            return sizeof(DAtomicType);
        }

        DAtomicType *
        DAtomicType::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            return mm.std_copy_for(this);
        }

        std::size_t
        DAtomicType::forward_children(obj<ACollector>) noexcept
        {
            return this->shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DAtomicType.cpp */
