/** @file stringtable2_register_facets.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "stringtable2_register_facets.hpp"

#include <xo/stringtable2/UniqueString.hpp>
#include <xo/stringtable2/String.hpp>

#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::print::APrintable;
    using xo::mm::AGCObject;
    using xo::scm::DString;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;

    namespace scm {
        bool
        stringtable2_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AGCObject, DUniqueString>();
            FacetRegistry::register_impl<APrintable, DUniqueString>();

            FacetRegistry::register_impl<AGCObject, DString>();
            FacetRegistry::register_impl<APrintable, DString>();

            log && log(xtag("DString.tseq", typeseq::id<DString>()));

            return true;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end stringtable2_register_facets.cpp */
