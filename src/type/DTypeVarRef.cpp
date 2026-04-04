/** @file DTypeVarRef.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "Metatype.hpp"
#include "TypeVarRef.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::facet::FacetRegistry;

    namespace scm {

        DTypeVarRef *
        DTypeVarRef::_make(obj<AAllocator> mm, const DUniqueString * name)
        {
            void * mem = mm.alloc_for<DTypeVarRef>();

            return new (mem) DTypeVarRef(name);
        }

        obj<AType,DTypeVarRef>
        DTypeVarRef::make(obj<AAllocator> mm, const DUniqueString * name)
        {
            return obj<AType,DTypeVarRef>(_make(mm, name));
        }

        // ----- Type facet -----

        Metatype
        DTypeVarRef::metatype() const noexcept
        {
            if (type_) {
                // resolved typevar reference

                return type_.metatype();
            } else {
                // type var reference, not yet resolved

                return Metatype::t_any();
            }
        }

        TypeDescr
        DTypeVarRef::repr_td() const noexcept
        {
            return Reflect::require<void *>();
        }

        bool
        DTypeVarRef::is_equal_to(const obj<AType> & y) const noexcept
        {
            // not really well-defined until types are resolved

            return (this->metatype().code() == y.metatype().code());
        }

        bool
        DTypeVarRef::is_subtype_of(const obj<AType> & y) const noexcept
        {
            Metatype x_mtype = this->metatype();
            Metatype y_mtype = y.metatype();

            if (y_mtype.code() == Metatype::code::t_any)
                return true;

            return (x_mtype.code() == y_mtype.code());
        }

        // ----- GCObject facet -----

        std::size_t
        DTypeVarRef::shallow_size() const noexcept
        {
            return sizeof(DTypeVarRef);
        }

        DTypeVarRef *
        DTypeVarRef::shallow_move(obj<ACollector> gc) noexcept
        {
            return gc.std_copy_for(this);
        }

        std::size_t
        DTypeVarRef::forward_children(obj<ACollector> gc) noexcept
        {
            gc.forward_pivot_inplace(&type_);

            //{
            //    auto e = FacetRegistry::instance().variant<AGCObject,AType>(type_);
            //    gc.forward_inplace(e.iface(), (void **)&type_.data_);
            //}

            return this->shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DTypeVarRef.cpp */
