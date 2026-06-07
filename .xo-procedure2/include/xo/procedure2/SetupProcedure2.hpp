/** @file SetupProcedure2.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "PrimitiveRegistry.hpp"
#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        struct SetupProcedure2 {
        public:
            using ACollector = xo::mm::ACollector;

        public:
            /** Register procedure2 (facet,impl) combinations with FacetRegistry **/
            static bool register_facets();
            /** Register gc-aware (AGCObject,DRepr) combinations with garbage collector @p gc **/
            static bool register_types(obj<xo::mm::ACollector> gc);
            static bool register_primitives(obj<ARuntimeContext> rcx,
                                            InstallSink sink,
                                            InstallFlags flags);
        };
    }
}

/* end SetupProcedure2.hpp */
