/** @file DAtomicType.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "AtomicType.hpp"
#include <xo/reflect/Reflect.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {

        DAtomicType *
        DAtomicType::_make(obj<AAllocator> mm, Metatype mtype)
        {
            void * mem = mm.alloc_for<DAtomicType>();

            return new (mem) DAtomicType(mtype);
        }

        obj<AType,DAtomicType>
        DAtomicType::make(obj<AAllocator> mm, Metatype mtype)
        {
            return obj<AType,DAtomicType>(_make(mm, mtype));
        }

        // ----- Type facet -----

        TypeDescr
        DAtomicType::repr_td() const noexcept
        {
            return Reflect::require<void *>();
        }

        bool
        DAtomicType::is_equal_to(const obj<AType> & y) const noexcept
        {
            return (metatype_.code() == y.metatype().code());
        }

        bool
        DAtomicType::is_subtype_of(const obj<AType> & y) const noexcept
        {
            Metatype x_mtype = metatype_;
            Metatype y_mtype = y.metatype();

            if (y_mtype.code() == Metatype::code::t_any)
                return true;

            return (x_mtype.code() == y_mtype.code());
        }

        // ----- GCObject facet -----

        std::size_t
        DAtomicType::shallow_size() const noexcept
        {
            return sizeof(DAtomicType);
        }

        DAtomicType *
        DAtomicType::shallow_move(obj<AAllocator> mm) noexcept
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
