/** @file MutationLogStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include <xo/gc/MutationLogStore.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::rng::random_seed;
    using xo::xtag;
    using xo::scope;

    namespace {

        struct Testcase {
        };

        static std::vector<Testcase> s_testcase_v = {

        };

        class MlsFixture {
        };
    }

    TEST_CASE("MutationLogStore-1", "[MutationLogStore]")
    {
        constexpr bool c_debug_flag = true;
        scope log0(XO_DEBUG(c_debug_flag), "MutationLogStore test");

        std::uint64_t seed = 7988747704879432247ul;
        //random_seed(&seed);
        log0 && log0(xtag("seed", seed));

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            auto rgen = xoshiro256ss(seed + i_tc);

            const Testcase & tc = s_testcase_v[i_tc];

            scope log1(XO_DEBUG(tc.debug_flag_), "testcase loop", xtag("i_tc", i_tc));

            INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

            MlsFixture fixture(tc);
        }

    }
} /*namespace ut*/

/* end MutationLogStore.test.cpp */
