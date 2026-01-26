/** @file procedure2_register_facets.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/gc/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register procedure2 (facet,impl) combinations with FacetRegistry **/
        bool procedure2_register_facets();
    }
}

/* end procedure2_register_facets.hpp */
