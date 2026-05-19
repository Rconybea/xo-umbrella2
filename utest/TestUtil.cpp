/** @file TestUtil.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "TestUtil.hpp"
#include <catch2/catch.hpp>

namespace xo {
    UtestConfig *
    UtestConfig::instance() {
        static UtestConfig s_instance;

        return &s_instance;
    };

    scope
    Utest::ut_scope() {
        return scope(XO_DEBUG(UtestConfig::instance()->debug_flag()),
                     xtag("name", Catch::getResultCapture().getCurrentTestName()));
    }
}

/* end TestUtil.cpp */
