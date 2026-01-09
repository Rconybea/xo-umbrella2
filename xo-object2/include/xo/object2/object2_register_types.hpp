/** @file object2_register_types.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/gc/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register all object2/ gc-aware types with @p gc.
         *  Return true iff all types register successfully.
         **/
        bool object2_register_types(obj<xo::mm::ACollector> gc);

        /** Register object2 (facet,impl) combinations with FacetRegistry **/
        bool object2_register_facets();
    }
}

/* end object2_register_types.hpp */
