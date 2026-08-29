/** @file setup_reactor2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "setup_reactor2.hpp"
#include "ObjectEvent.hpp"
#include <xo/alloc2/GCObject.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/reflectutil/typeseq.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::pp::scope;
    using xo::pp::xtag;
    using xo::reflect::typeseq;

    namespace process {
        bool
        SetupReactor2::register_facets()
        {
            scope log(XO_DEBUG_(true));

            FacetRegistry::register_impl<AGCObject, DObjectEvent>();
            FacetRegistry::register_impl<APrintable, DObjectEvent>();

            log && log(xtag("DObjectEvent.tseq", typeseq::id<DObjectEvent>()));

            log && log(xtag("APrintable.tseq", typeseq::id<APrintable>()));
            log && log(xtag("AGCObject.tseq", typeseq::id<AGCObject>()));

            return true;
        }
    }
} /*namespace xo*/

/* setup_reactor2.cpp */
