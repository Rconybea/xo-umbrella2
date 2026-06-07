/** @file SetupStringtable2.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "SetupStringtable2.hpp"

#include <xo/stringtable2/UniqueString.hpp>
#include <xo/stringtable2/String.hpp>

#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::print::APrintable;
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::scm::DString;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;
    using xo::facet::impl_for;

    namespace scm {
        bool
        SetupStringtable2::register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AGCObject, DUniqueString>();
            FacetRegistry::register_impl<APrintable, DUniqueString>();

            FacetRegistry::register_impl<AGCObject, DString>();
            FacetRegistry::register_impl<APrintable, DString>();

            log && log(xtag("DString.tseq", typeseq::id<DString>()));

            return true;
        }

        bool
        SetupStringtable2::register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            ok &= gc.install_type(impl_for<AGCObject, DUniqueString>());
            ok &= gc.install_type(impl_for<AGCObject, DString>());

            return ok;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end SetupStringtable2.cpp */
