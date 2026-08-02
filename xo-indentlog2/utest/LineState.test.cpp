/** @file LineState.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "LineState.hpp"
#include <xo/testutil/try_test_array.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <xo/randomgen/xoshiro256.hpp>
// #include <xo/testutil/UtestRehearser.hpp>
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

    vector<TestCase_LineState>
    s_linestate_testcase_v = {
        /* reminder:
         *  LineState(local_ppos, solpos, color_escape_chars, color_escape_start)
         */

        /* test case: trivial: 0 chars written from empty string */
        TestCase_LineState("",
                           {{0, LineState(0, 0, 0, -1)}
                           }),
        /* test case: 1 non-special character */
        TestCase_LineState("a",
                           {{0, LineState(0, 0, 0, -1)},
                            {1, LineState(1, 0, 0, -1)}}),
        /* test case: 1 special character (newline) */
        TestCase_LineState("\n",
                           {{0, LineState(0, 0, 0, -1)},
                            {1, LineState(1, 1, 0, -1)}}),
        /* test case: 1 special character (carriage return) */
        TestCase_LineState("\r",
                           {{0, LineState(0, 0, 0, -1)},
                            {1, LineState(1, 1, 0, -1)}}),
        /* test case: 1 special character (escape)
         *  (defer counting color-escape chars until the end of the escape)
         */
        TestCase_LineState("\033",
                           {{0, LineState(0, 0, 0, -1)},
                            {1, LineState(1, 0, 0,  0)}}),

        /* test case: color-escape \033..m
         *   (defer counting color-escape chars until the end of the escape)
         */
        TestCase_LineState("\033[31;9m",
                           {{0, LineState(0, 0, 0, -1)},
                            {6, LineState(6, 0, 0,  0)},
                            {7, LineState(7, 0, 7, -1)}}),

        /* test case: incomplete color-escape (interrupted by non-escape-eligible char) */
        TestCase_LineState("\033[31;hello",
                           {{ 0, LineState( 0, 0, 0, -1)},
                            { 5, LineState( 5, 0, 0,  0)},
                            { 6, LineState( 6, 0, 0, -1)},
                            {10, LineState(10, 0, 0, -1)}}),

        /* test case: multiple lines, no color escapes */
        TestCase_LineState("we hold\nall these truths\nself-evident",
                           {{ 0, LineState( 0,  0, 0, -1)},
                            { 7, LineState( 7,  0, 0, -1)},
                            { 8, LineState( 8,  8, 0, -1)},
                            {24, LineState(24,  8, 0, -1)},
                            {25, LineState(25, 25, 0, -1)},
                            {37, LineState(37, 25, 0, -1)},
                           }),

        /* test case: multiple line, color escapes */
        TestCase_LineState("we \033[31;9mhold\n\033[0mall \033[31;13mthese\033[0m truths\nself-evident",
                           {{ 0, LineState( 0,  0,  0, -1)},
                            /* color-escape \033..m on 1st line uses 7 chars */
                            {14, LineState(14,  0,  7, -1)},
                            /* \n at char 15 resets color_escape */
                            {15, LineState(15, 15,  0, -1)},
                            /* color escape \033..m on 2nd line uses 4 chars */
                            {20, LineState(20, 15,  4, -1)},
                            /* 2nd color escape on 2nd line uses 8 chars */
                            {32, LineState(32, 15, 12, -1)},
                            /* incomplete 3rd color escape at this point: */
                            {39, LineState(39, 15, 12, 36)},
                            /* 3rd color escape on 2nd line uses 4 chars */
                            {40, LineState(40, 15, 16, -1)},
                            /* \n at 47 resets color escape */
                            {48, LineState(48, 48,  0, -1)},
                            {60, LineState(60, 48,  0, -1)},
                           }),

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
