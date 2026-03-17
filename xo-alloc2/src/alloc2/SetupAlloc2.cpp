/** @file SetupAlloc2.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "SetupAlloc2.hpp"
#include <xo/alloc2/Arena.hpp>
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

            log && log(xtag("DArena.tseq", typeseq::id<DArena>()));

            return true;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end SetupAlloc2.cpp */
