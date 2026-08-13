/** @file setup_gc.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "setup_gc.hpp"
#include "GCObjectStoreVisitor.hpp"
#include "X1Collector.hpp"
#include "X1CollectorIterator.hpp"
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>

namespace xo {
    /* the ppsink logging vocabulary, for use below.  Converted from legacy
     * xo::scope / xo::xtag 2026-08-13; see
     * .xo-backlog/xo-gc/issues/01-gc-free-of-indentlog.md
     */
    using xo::pp::scope;
    using xo::pp::xtag;

    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::DX1Collector;
    using xo::facet::FacetRegistry;
    using xo::reflect::typeseq;

    namespace mm {

        bool
        SetupGc::register_facets()
        {
            scope log(XO_DEBUG_(true));

            FacetRegistry::register_impl<AAllocator, DX1Collector>();
            FacetRegistry::register_impl<ACollector, DX1Collector>();
            FacetRegistry::register_impl<AAllocIterator, DX1CollectorIterator>();

            FacetRegistry::register_impl<AGCObjectVisitor, DGCObjectStoreVisitor>();

            log && log(xtag("DX1Collector.tseq", typeseq::id<DX1Collector>()));
            log && log(xtag("DX1CollectorIterator.tseq", typeseq::id<DX1CollectorIterator>()));
            log && log(xtag("DGCObjectStoreVisitor.tseq", typeseq::id<DGCObjectStoreVisitor>()));

            log && log(xtag("ACollector.tseq",  typeseq::id<ACollector>()));
            log && log(xtag("AGCObjectVisitor.tseq",  typeseq::id<AGCObjectVisitor>()));

            return true;
        }
    }
} /*namespace xo*/

/* end setup_gc.cpp */
