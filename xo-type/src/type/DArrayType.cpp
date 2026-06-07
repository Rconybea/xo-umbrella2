/** @file DArrayType.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "Type.hpp"
#include "ArrayType.hpp"
#include "TypeDescr.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::mm::AGCObject;
    using xo::mm::AAllocator;
    using xo::facet::FacetRegistry;

    namespace scm {

        DArrayType::DArrayType(obj<AType> elt)
            : elt_type_{elt}
        {}

        DArrayType *
        DArrayType::_make(obj<AAllocator> mm,
                          obj<AType> elt_type)
        {
            void * mem = mm.alloc_for<DArrayType>();

            return new (mem) DArrayType(elt_type);
        }

        // ----- type facet -----

        TypeDescr
        DArrayType::repr_td() const noexcept
        {
            return Reflect::require<void *>();
        }

        bool
        DArrayType::is_equal_to(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype != Metatype::t_array())
                return false;

            auto y = obj<AType,DArrayType>::from(y_arg);

            obj<AType> e = elt_type_;

            return (e.is_equal_to(y->elt_type_));
        }

        bool
        DArrayType::is_subtype_of(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype == Metatype::t_any())
                return true;

            if (y_mtype != Metatype::t_array())
                return false;

            auto y = obj<AType,DArrayType>::from(y_arg);

            obj<AType> e = elt_type_;

            return (e.is_subtype_of(y->elt_type_));
        }

        // ----- gcobject facet -----

        DArrayType *
        DArrayType::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DArrayType::visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_poly_child(reason, &elt_type_);
        }
    }
}

/* end DArrayType.cpp */
