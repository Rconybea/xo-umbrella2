/** @file try_test_array.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "UtestRehearser.hpp"
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tostr.hpp>
#include <catch2/catch.hpp>
#include <cstddef>
#include <vector>

namespace xo {
    /** e.g. RehearseFn = bool (*)(const TestCase &, UtestRehearser *) **/
    template<typename TestCase, typename RehearseFn>
    void try_test_array(const std::vector<TestCase> & tc_v,
                        RehearseFn rehearse_fn)
    {
        using xo::pp::tostr;
        using xo::pp::xtag;

        for (size_t i_tc = 0, n_tc = tc_v.size(); i_tc < n_tc; ++i_tc)
        {
            const auto & tc = tc_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc)));

            UtestRehearser rh;

            for (auto _ : rh) {
                /* always do 1st loop.
                 * if any test assertions fail, do 2nd loop with verbose logging
                 */

                (*rehearse_fn)(tc, &rh);
            }
        }
    }
} /*namespace xo*/

/* end try_test_array.hpp */
