/** @file DListType.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "Type.hpp"
#include "ListType.hpp"
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
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

        // ----- type facet -----

        bool
        DListType::is_equal_to(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype != Metatype::list())
                return false;

            auto y = obj<AType,DListType>::from(y_arg);

            obj<AType> e = elt_type_;

            return (e.is_equal_to(y->elt_type_));
        }

        bool
        DListType::is_subtype_of(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype == Metatype::any())
                return true;

            if (y_mtype != Metatype::list())
                return false;

            auto y = obj<AType,DListType>::from(y_arg);

            obj<AType> e = elt_type_;

            return (e.is_subtype_of(y->elt_type_));
        }

        // ----- gcobject facet -----

        std::size_t
        DListType::shallow_size() const noexcept
        {
            return sizeof(*this);
        }

        DListType *
        DListType::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            return mm.std_copy_for(this);
        }

        std::size_t
        DListType::forward_children(obj<ACollector> gc) noexcept
        {
            {
                auto e = FacetRegistry::instance().variant<AGCObject,AType>(elt_type_);
                gc.forward_inplace(e.iface(), (void **)&(e.data_));
            }

            return this->shallow_size();
        }
    }
}

/* end DListType.cpp */
