/** @file TestUtil.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

#include <xo/indentlog/scope.hpp>

namespace xo {

    /** unit-test configuration here
     *
     *  TODO: promote to its own library, along with UtestListener
     **/
    struct UtestConfig {
        bool debug_flag() const { return debug_flag_; }

        /** announce each test using catch2's listener api **/
        bool announce_flag_ = false;
        /** enable debug output for all (!) tests **/
        bool debug_flag_ = false;

        static UtestConfig * instance();
    };

    /** RAII logging for catch unit tests
     *
     *  Use:
     *    TEST_CASE(name, tags, ..)
     *    {
     *       scope log = Utest::ut_scope();
     *
     *       ...
     *       log && log(xtag("foo", ...));
     *    }
     **/
    struct Utest {
        static scope ut_scope();
    };

} /*namespace xo*/

/* end TestUtil.hpp */
