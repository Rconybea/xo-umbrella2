/** @file SetupType.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "SetupType.hpp"
#include "ArrayType.hpp"
#include "AtomicType.hpp"
#include "FunctionType.hpp"
#include "ListType.hpp"
#include "TypeVarRef.hpp"
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>

namespace xo {
    /* the ppsink logging vocabulary, for use below.  Was reaching legacy
     * xo::scope / xo::xtag through a TRANSITIVE xo-indentlog propagated by
     * xo-object2, which dropped that dependency at phase E; xo-type never
     * declared one itself (its find_dependency is commented out).
     */
    using xo::pp::scope;

    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    using xo::facet::impl_for;
    using xo::reflect::typeseq;

    namespace scm {

        bool
        SetupType::register_facets()
        {
            scope log(XO_DEBUG_(true));

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

            log && log(xo::pp::xtag("DAtomicType.tseq", typeseq::id<DAtomicType>()));
            log && log(xo::pp::xtag("DListType.tseq", typeseq::id<DListType>()));
            log && log(xo::pp::xtag("DArrayType.tseq", typeseq::id<DArrayType>()));
            log && log(xo::pp::xtag("DFunctionType.tseq", typeseq::id<DFunctionType>()));
            log && log(xo::pp::xtag("DTypeVarRef.tseq", typeseq::id<DTypeVarRef>()));

            return true;
        }

        bool
        SetupType::register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG_(true));

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
