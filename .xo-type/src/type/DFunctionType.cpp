/** @file DFunctionType.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "FunctionType.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::facet::FacetRegistry;

    namespace scm {

        // ----- type facet -----

        TypeDescr
        DFunctionType::repr_td() const noexcept
        {
            return Reflect::require<void *>();
        }

        bool
        DFunctionType::is_equal_to(obj<AType> y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype != Metatype::t_function())
                return false;

            auto y = obj<AType,DFunctionType>::from(y_arg);
            assert(y);

            if (arg_types_->size() != y->arg_types_->size())
                return false;

            if (obj<AType>(return_type_).is_equal_to(y->return_type_))
                return false;

            for (size_t i = 0, n = arg_types_->size(); i < n; ++i) {
                auto argtype1
                    = FacetRegistry::instance().variant<AType,AGCObject>((*arg_types_)[i]);
                auto argtype2
                    = FacetRegistry::instance().variant<AType,AGCObject>((*(y->arg_types_))[i]);

                if (argtype1.is_equal_to(argtype2))
                    return false;
            }

            return true;
        }

        bool
        DFunctionType::is_subtype_of(const obj<AType> & y_arg) const noexcept
        {
            Metatype y_mtype = y_arg.metatype();

            if (y_mtype == Metatype::t_any())
                return true;

            if (y_mtype != Metatype::t_function())
                return false;

            auto y = obj<AType,DFunctionType>::from(y_arg);
            assert(y);

            if (arg_types_->size() != y->arg_types_->size())
                return false;

            if (!obj<AType>(return_type_).is_subtype_of(y->return_type_))
                return false;

            for (size_t i = 0, n = arg_types_->size(); i < n; ++i) {
                auto x_argtype
                    = FacetRegistry::instance().variant<AType,AGCObject>((*arg_types_)[i]);
                auto y_argtype
                    = FacetRegistry::instance().variant<AType,AGCObject>((*(y->arg_types_))[i]);

                if (!y_argtype.is_subtype_of(x_argtype))
                    return false;
            }

            return true;
        }

        // ----- gcobject facet -----

        DFunctionType *
        DFunctionType::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DFunctionType::visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_poly_child(reason, &return_type_);
            gc.visit_child(reason, &arg_types_);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DFunctionType.cpp */
