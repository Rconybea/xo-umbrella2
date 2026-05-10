/** @file SetupAlloc2.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "SetupAlloc2.hpp"
#include <xo/alloc2/Arena.hpp>
#include <xo/alloc2/ArenaIterator.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::facet::FacetRegistry;
    //using xo::facet::TypeRegistry;
    using xo::reflect::typeseq;

    namespace mm {

        bool
        SetupAlloc2::register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AAllocator, DArena>();
            FacetRegistry::register_impl<AAllocIterator, DArenaIterator>();

            log && log(xtag("DArena.tseq", typeseq::id<DArena>()));
            log && log(xtag("DArenaIterator.tseq", typeseq::id<DArenaIterator>()));

            log && log(xtag("AAllocator.tseq", typeseq::id<AAllocator>()));
            log && log(xtag("AAllocIterator.tseq", typeseq::id<AAllocIterator>()));

            return true;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end SetupAlloc2.cpp */
