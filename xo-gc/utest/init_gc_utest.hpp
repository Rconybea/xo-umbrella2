/** @file init_gc_utest.hpp
*
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent xo-gc/utest/ as a mock subsystem within ordered initialization
     *
     * (so we can follow usual patterns for setting up facet tables)
     */
    enum S_gc_utest_tag {};

    template <>
    struct InitSubsys<S_gc_utest_tag> {
        static void init();
        static InitEvidence require();
    };

    namespace mm {

        class SetupGcUtest {
        public:
            /** Register gc/utest (facet,impl) combinations with FacetRegistry **/
            static bool register_facets();
        };
    } /*namespace mm*/

} /*namespace xo*/

/* end init_gc_utest.hpp */
