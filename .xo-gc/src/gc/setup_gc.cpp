/** @file setup_gc.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "setup_gc.hpp"
#include "X1Collector.hpp"
#include "X1CollectorIterator.hpp"
#include "GCObjectStoreVisitor.hpp"
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::DX1Collector;
    using xo::facet::FacetRegistry;
    using xo::reflect::typeseq;

    namespace mm {

        bool
        SetupGc::register_facets()
        {
            scope log(XO_DEBUG(true));

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
