/** @file SetupAlloc2.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

namespace xo {
    namespace mm {
        class SetupAlloc2 {
        public:
            /** Register alloc2 (facet,impl) combinations with Facet Registry **/
            static bool register_facets();
        };
    }
}

/* end SetupAlloc2.hpp */
