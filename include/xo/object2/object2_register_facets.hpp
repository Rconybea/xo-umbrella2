/** @file object2_register_facets.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/gc/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register object2 (facet,impl) combinations with FacetRegistry **/
        bool object2_register_facets();
    }
}

/* end object2_register_facets.hpp */
