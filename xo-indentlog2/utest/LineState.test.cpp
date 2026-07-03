/** @file LineState.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "LineState.hpp"
//#include <xo/testutil/
#include <xo/randomgen/random_seed.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <catch2/catch.hpp>
#include <vector>

namespace ut {
    using xo::LineState;
    using xo::rng::Seed;
    using xo::rng::xoshiro256ss;
    using xo::UtestRehearser;
    using std::vector;
    using std::uint32_t;

    /** Unit test setup.
     *  Test sequence:
     *  call LineState::_check_update_localstate()
     *  with progressive prefixes comprising the first break_v_[i].first
     *  characters in text_; expect resulting state to match break_v_[i].second
     **/
    struct TestCase_LineState {
        TestCase_LineState(const char * text,
                           std::vector<std::pair<uint32_t, LineState>> break_v)
            : text_{text},
              break_v_{std::move(break_v)}
            {}

        const char * text_ = nullptr;
        std::vector<std::pair<uint32_t, LineState>> break_v_ = {};
    };

    /** e.g. RehearseFn = bool (*)(const TestCase &, UtestRehearser *) **/
    template<typename TestCase, typename RehearseFn>
    void try_test_array(const vector<TestCase> & tc_v,
                        RehearseFn rehearse_fn)
    {
        for (size_t i_tc = 0, n_tc = tc_v.size(); i_tc < n_tc; ++i_tc)
        {
            const auto & tc = tc_v[i_tc];

            Seed<xoshiro256ss> seed;
            auto rgen = xoshiro256ss(seed);

            UtestRehearser rh;

            for (auto _ : rh) {
                /** always do 1st loop.
                 *  if any test assertions fail, do 2nd loop with verbose logging
                 **/

                (*rehearse_fn)(tc, &rh);
            }
        }
    }

    vector<TestCase_LineState>
    s_linestate_testcase_v = {
        /* 1st test case:
         *   trivial: 0 chars written from empty string
         */
        TestCase_LineState("",
                           {{0, LineState(0 /*local_ppos*/,
                                          0 /*solpos*/,
                                          0 /*color_escape_chars*/,
                                          -1 /*color_escape_start*/)}
                           }),
        TestCase_LineState("a",
                           {{0, LineState(0 /*local_ppos*/,
                                          0 /*solpos*/,
                                          0 /*color_escape_chars*/,
                                          -1 /*color_escape_start*/)},
                            {1, LineState(1 /*local_ppos*/,
                                          0 /*solpos*/,
                                          0 /*color_escape_chars*/,
                                          -1 /*color_escape_start*/)}}),
    };

    void
    linestate_test_fn(const TestCase_LineState & tc,
                      UtestRehearser * p_rh)
    {
        Seed<xoshiro256ss> seed;
        auto rgen = xoshiro256ss(seed);

        LineState act_lstate;

        for (uint32_t j_prefix = 0; j_prefix < tc.break_v_.size(); ++j_prefix) {
            act_lstate._check_update_local_state
                (tc.text_,
                 tc.text_ + tc.break_v_[j_prefix].first,
                 p_rh->enable_debug());

            const LineState & exp_lstate = tc.break_v_[j_prefix].second;

            REHEARSE(*p_rh,
                     act_lstate.local_ppos() == exp_lstate.local_ppos());
            REHEARSE(*p_rh,
                     act_lstate.solpos() == exp_lstate.solpos());
            REHEARSE(*p_rh,
                     act_lstate.color_escape_chars()
                     == exp_lstate.color_escape_chars());
            REHEARSE(*p_rh,
                     act_lstate.color_escape_start()
                     == exp_lstate.color_escape_start());
        }
    }

    TEST_CASE("LineState", "[LineState]")
    {
        //constexpr bool c_debug_flag = false;

        try_test_array(s_linestate_testcase_v,
                       &linestate_test_fn);
    }
}

/* end LineState.test.cpp */
