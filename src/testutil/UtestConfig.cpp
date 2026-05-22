/** @file UtestConfig.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "UtestConfig.hpp"
#include <catch2/catch.hpp>

namespace xo {
    UtestConfig *
    UtestConfig::instance() {
        static UtestConfig s_instance;

        return &s_instance;
    }
}

/* end UtestConfig.cpp */
