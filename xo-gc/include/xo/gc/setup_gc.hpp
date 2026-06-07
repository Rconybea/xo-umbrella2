/** @file SetupGc.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

namespace xo {
    namespace mm {

        class SetupGc {
        public:
            /** Register gc (facet,impl) combinations with FacetRegistry **/
            static bool register_facets();
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end SetupGc.hpp */
