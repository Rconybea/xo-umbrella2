/** @file SetupType.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        struct SetupType {
        public:
            using ACollector = xo::mm::ACollector;

        public:
            /** Register type (facet,impl) combinations with FacetRegistry **/
            static bool register_facets();
            /** Register type (facet,impl) combinations with FacetRegistry **/
            static bool register_types(obj<ACollector> gc);
        };
    }
}

/* end SetupType.hpp */
