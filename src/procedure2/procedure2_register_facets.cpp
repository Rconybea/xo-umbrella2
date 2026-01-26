/** @file procedure2_register_facets.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DSimpleRcx.hpp"
#include "detail/IRuntimeContext_DSimpleRcx.hpp"

#include "DPrimitive_gco_2_gco_gco.hpp"
#include "detail/IGCObject_DPrimitive_gco_2_gco_gco.hpp"
#include "detail/IPrintable_DPrimitive_gco_2_gco_gco.hpp"

#include <xo/gc/GCObject.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;
    using xo::print::APrintable;

    namespace scm {
        bool
        procedure2_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<ARuntimeContext, DSimpleRcx>();

            FacetRegistry::register_impl<AGCObject, DPrimitive_gco_2_gco_gco>();
            FacetRegistry::register_impl<APrintable, DPrimitive_gco_2_gco_gco>();

            log && log(xtag("DSimpleRcx.tseq", typeseq::id<DSimpleRcx>()));
            log && log(xtag("DPrimitive_gco_2_gco_gco.tseq", typeseq::id<DPrimitive_gco_2_gco_gco>()));

            return true;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end procedure2_register_facets.cpp */
