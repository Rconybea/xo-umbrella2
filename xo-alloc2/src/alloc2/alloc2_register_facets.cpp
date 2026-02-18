/** @file alloc2_register_facets.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "alloc2_register_facets.hpp"
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::facet::FacetRegistry;
    //using xo::facet::TypeRegistry;
    using xo::reflect::typeseq;

    namespace mm {

        bool
        alloc2_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AAllocator, DArena>();

            log && log(xtag("DArena.tseq", typeseq::id<DArena>()));

            return true;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end alloc2_register_facets.cpp */
