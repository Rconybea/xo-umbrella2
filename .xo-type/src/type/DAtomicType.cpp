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

        DAtomicType *
        DAtomicType::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DAtomicType::visit_gco_children(VisitReason, obj<AGCObjectVisitor>) noexcept
        {
            // no-op.  no children
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DAtomicType.cpp */
