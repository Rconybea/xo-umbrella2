/** @file SetupObject2.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "SetupObject2.hpp"
#include "RuntimeError.hpp"
#include <xo/object2/Array.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/object2/Dictionary.hpp>
#include <xo/object2/Float.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/List.hpp>
#include <xo/stringtable2/String.hpp>
#include <xo/alloc2/alloc/AAllocator.hpp>
#include <xo/printable2/detail/APrintable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>

namespace xo {
    /* the ppsink logging vocabulary, for use below */
    using xo::pp::scope;

    using xo::print::APrintable;
    using xo::mm::ACollector;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::scm::DList;
    using xo::scm::DBoolean;
    using xo::scm::DFloat;
    using xo::scm::DString;
    using xo::scm::DArray;
    using xo::facet::DVariantPlaceholder;
    using xo::facet::FacetRegistry;
    using xo::facet::impl_for;
    using xo::facet::typeseq;

    namespace scm {
        bool
        SetupObject2::register_facets()
        {
            scope log(XO_DEBUG_(true));

            FacetRegistry::register_impl<AGCObject, DBoolean>();
            FacetRegistry::register_impl<APrintable, DBoolean>();

            FacetRegistry::register_impl<AGCObject, DFloat>();
            FacetRegistry::register_impl<APrintable, DFloat>();

            FacetRegistry::register_impl<AGCObject, DInteger>();
            FacetRegistry::register_impl<APrintable, DInteger>();

            FacetRegistry::register_impl<AGCObject, DString>();
            FacetRegistry::register_impl<APrintable, DString>();

            FacetRegistry::register_impl<AGCObject, DList>();
            FacetRegistry::register_impl<APrintable, DList>();
            FacetRegistry::register_impl<ASequence, DList>();

            FacetRegistry::register_impl<AGCObject, DArray>();
            FacetRegistry::register_impl<APrintable, DArray>();
            FacetRegistry::register_impl<ASequence, DArray>();

            FacetRegistry::register_impl<AGCObject, DDictionary>();
            FacetRegistry::register_impl<APrintable, DDictionary>();

            FacetRegistry::register_impl<AGCObject, DRuntimeError>();
            FacetRegistry::register_impl<APrintable, DRuntimeError>();

            log && log(xo::pp::xtag("DVariantPlaceholder.tseq", typeseq::id<DVariantPlaceholder>()));

            log && log(xo::pp::xtag("DList.tseq", typeseq::id<DList>()));
            log && log(xo::pp::xtag("DBoolean.tseq", typeseq::id<DBoolean>()));
            log && log(xo::pp::xtag("DFloat.tseq", typeseq::id<DFloat>()));
            log && log(xo::pp::xtag("DInteger.tseq", typeseq::id<DInteger>()));
            log && log(xo::pp::xtag("DArray.tseq", typeseq::id<DArray>()));
            log && log(xo::pp::xtag("DDictionary.tseq", typeseq::id<DDictionary>()));
            log && log(xo::pp::xtag("DRuntimeError.tseq", typeseq::id<DRuntimeError>()));

            log && log(xo::pp::xtag("AAllocator.tseq", typeseq::id<AAllocator>()));
            log && log(xo::pp::xtag("APrintable.tseq", typeseq::id<APrintable>()));
            log && log(xo::pp::xtag("AGCObject.tseq", typeseq::id<AGCObject>()));
            log && log(xo::pp::xtag("ASequence.tseq", typeseq::id<ASequence>()));

            return true;
        }

        bool
        SetupObject2::register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG_(true));

            bool ok = true;

            ok &= gc.install_type(impl_for<AGCObject, DBoolean>());
            ok &= gc.install_type(impl_for<AGCObject, DFloat>());
            ok &= gc.install_type(impl_for<AGCObject, DInteger>());
            ok &= gc.install_type(impl_for<AGCObject, DList>());
            ok &= gc.install_type(impl_for<AGCObject, DArray>());
            ok &= gc.install_type(impl_for<AGCObject, DDictionary>());
            ok &= gc.install_type(impl_for<AGCObject, DRuntimeError>());

            return ok;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end SetupObject2.cpp */
