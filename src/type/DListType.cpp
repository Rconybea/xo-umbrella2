/** @file DListType.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "Type.hpp"
#include "ListType.hpp"
#include "TypeDescr.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::mm::AGCObject;
    using xo::mm::AAllocator;
    using xo::facet::FacetRegistry;

    namespace scm {

        DListType::DListType(obj<AType> elt) : elt_type_{elt} {}

        DListType *
        DListType::_make(obj<AAllocator> mm,
                         obj<AType> elt_type)
        {
            void * mem = mm.alloc_for<DListType>();

            return new (mem) DListType(elt_type);
        }

        obj<AType, DListType>
        DListType::make(obj<AAllocator> mm,
                        obj<AType> elt_type)
        {
            return obj<AType, DListType>(_make(mm, elt_type));
        }

        // ----- type facet -----

        TypeDescr
        DListType::repr_td() const noexcept
        {
            return Reflect::require<void *>();
        }

        bool
        DListType::is_equal_to(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype != Metatype::t_list())
                return false;

            auto y = obj<AType,DListType>::from(y_arg);

            obj<AType> e = elt_type_;

            return (e.is_equal_to(y->elt_type_));
        }

        bool
        DListType::is_subtype_of(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype == Metatype::t_any())
                return true;

            if (y_mtype != Metatype::t_list())
                return false;

            auto y = obj<AType,DListType>::from(y_arg);

            obj<AType> e = elt_type_;

            return (e.is_subtype_of(y->elt_type_));
        }

        // ----- gcobject facet -----

        DListType *
        DListType::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DListType::visit_gco_children(VisitReason reason,
                                      obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_poly_child(reason, &elt_type_);
            //{
            //    auto e = FacetRegistry::instance().variant<AGCObject,AType>(elt_type_);
            //    gc.forward_inplace(e.iface(), (void **)&(elt_type_.data_));
            //}
        }
    }
}

/* end DListType.cpp */
