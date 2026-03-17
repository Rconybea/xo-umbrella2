/** @file SetupStringtable2.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        struct SetupStringtable2 {
        public:
            using ACollector = xo::mm::ACollector;

        public:
            /** Register object2 (facet,impl) combinations with FacetRegistry **/
            static bool register_facets();
            /** Register types with garbage collector **/
            static bool register_types(obj<ACollector> gc);
        };
    }
}

/* end SetupStringtable2.hpp */
