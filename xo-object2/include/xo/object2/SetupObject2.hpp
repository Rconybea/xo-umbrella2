/** @file SetupObject2.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        struct SetupObject2 {
        public:
            using ACollector = xo::mm::ACollector;

        public:
            static bool register_facets();
            static bool register_types(obj<ACollector> gc);
        };
    }
}

/* end object2_register_facets.hpp */
