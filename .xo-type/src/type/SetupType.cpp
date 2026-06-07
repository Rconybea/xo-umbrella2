/** @file SetupType.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "SetupType.hpp"
#include "AtomicType.hpp"
#include "ListType.hpp"
#include "ArrayType.hpp"
#include "FunctionType.hpp"
#include "TypeVarRef.hpp"

#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    using xo::facet::impl_for;
    using xo::reflect::typeseq;

    namespace scm {

        bool
        SetupType::register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AType, DAtomicType>();
            FacetRegistry::register_impl<AGCObject, DAtomicType>();

            FacetRegistry::register_impl<AType, DListType>();
            FacetRegistry::register_impl<AGCObject, DListType>();

            FacetRegistry::register_impl<AType, DArrayType>();
            FacetRegistry::register_impl<AGCObject, DArrayType>();

            FacetRegistry::register_impl<AType, DFunctionType>();
            FacetRegistry::register_impl<AGCObject, DFunctionType>();

            FacetRegistry::register_impl<AType, DTypeVarRef>();
            FacetRegistry::register_impl<AGCObject, DTypeVarRef>();

            log && log(xtag("DAtomicType.tseq", typeseq::id<DAtomicType>()));
            log && log(xtag("DListType.tseq", typeseq::id<DListType>()));
            log && log(xtag("DArrayType.tseq", typeseq::id<DArrayType>()));
            log && log(xtag("DFunctionType.tseq", typeseq::id<DFunctionType>()));
            log && log(xtag("DTypeVarRef.tseq", typeseq::id<DTypeVarRef>()));

            return true;
        }

        bool
        SetupType::register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            ok &= gc.install_type(impl_for<AGCObject, DAtomicType>());
            ok &= gc.install_type(impl_for<AGCObject, DListType>());
            ok &= gc.install_type(impl_for<AGCObject, DArrayType>());
            ok &= gc.install_type(impl_for<AGCObject, DFunctionType>());
            ok &= gc.install_type(impl_for<AGCObject, DTypeVarRef>());

            return ok;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end SetupType.cpp */
