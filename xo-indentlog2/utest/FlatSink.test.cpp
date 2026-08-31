/** @file FlatSink.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "init_indentlog2.hpp"
#include "print/PrettySink.hpp"
#include <xo/testutil/try_test_array.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/subsys/Subsystem.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp>
#include <catch2/catch.hpp>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::PrettySink;
    using xo::pp::PpSink;
    using xo::pp::PpConfig;
    using xo::S_indentlog2_tag;
    using xo::InitSubsys;
    using xo::InitEvidence;
    using xo::UtestRehearser;
    using std::string;
    using std::vector;

    static InitEvidence s_init = InitSubsys<S_indentlog2_tag>::require();

    TEST_CASE("DFlatSink-init", "[indentlog2][DFlatSink]")
    {
        REQUIRE(s_init.evidence());
    }

    /** one action in a PpSink scenario **/
    struct Step_FlatSink {
        enum Kind { k_put, k_stream, k_begin, k_split, k_end };

        static Step_FlatSink put(string s)    { return Step_FlatSink{k_put, std::move(s)}; }
        static Step_FlatSink stream(string s) { return Step_FlatSink{k_stream, std::move(s)}; }
        static Step_FlatSink begin() { return Step_FlatSink{k_begin, ""}; }
        static Step_FlatSink split() { return Step_FlatSink{k_split, ""}; }
        static Step_FlatSink end()   { return Step_FlatSink{k_end, ""}; }

        Kind kind_ = k_put;
        string text_;
    };

    /** drive @p step_v through @p sink using only the PpSink interface,
     *  so the same sequence can be replayed against any sink.
     **/
    static void
    flatsink_drive(PpSink & sink, const vector<Step_FlatSink> & step_v)
    {
        for (const Step_FlatSink & step : step_v) {
            switch (step.kind_) {
            case Step_FlatSink::k_put:
                sink.put(step.text_);
                break;
            case Step_FlatSink::k_stream:
                {
                    auto ins = sink.stream_open(1 /*min_z*/);
                    ins << step.text_;
                }   // <- inserter dtor commits
                break;
            case Step_FlatSink::k_begin:
                sink.begin();
                break;
            case Step_FlatSink::k_split:
                sink.split();
                break;
            case Step_FlatSink::k_end:
                sink.end();
                break;
            }
        }
    }

    /** Unit test setup.
     *  Test sequence:
     *  1. drive step_v_ through a FlatSink; expect exactly exp_output_.
     *  2. replay the same steps through a PrettySink whose margin is wide
     *     enough that every group fits; expect the same output.
     *     (a fitting group collapses its splits, so pretty == flat)
     **/
    struct TestCase_FlatSink {
        vector<Step_FlatSink> step_v_;
        string exp_output_;
    };

    vector<TestCase_FlatSink>
    s_flatsink_testcase_v = {
        /* bare text */
        { { Step_FlatSink::put("hello") }, "hello" },

        /* split outside any group is a no-op for both sinks */
        { { Step_FlatSink::put("foo,"), Step_FlatSink::split(), Step_FlatSink::put("bar") }, "foo,bar" },

        /* group whose split collapses (fits) */
        { { Step_FlatSink::begin(), Step_FlatSink::put("foo,"), Step_FlatSink::split(), Step_FlatSink::put("bar"), Step_FlatSink::end() },
          "foo,bar" },

        /* nested groups: structure discarded flat, collapsed when it fits */
        { { Step_FlatSink::begin(), Step_FlatSink::put("a"),
            Step_FlatSink::begin(), Step_FlatSink::put("b"), Step_FlatSink::split(), Step_FlatSink::put("c"), Step_FlatSink::end(),
            Step_FlatSink::put("d"), Step_FlatSink::end() },
          "abcd" },

        /* single streamed value via stream_open + inserter */
        { { Step_FlatSink::stream("x") }, "x" },

        /* streamed values interleaved with put() */
        { { Step_FlatSink::put("["), Step_FlatSink::stream("x"), Step_FlatSink::put("]") }, "[x]" },

        /* consecutive streamed values (sink reuse on both impls) */
        { { Step_FlatSink::stream("foo"), Step_FlatSink::stream("bar") }, "foobar" },

        /* streamed values inside a fitting group */
        { { Step_FlatSink::begin(), Step_FlatSink::stream("foo,"), Step_FlatSink::split(), Step_FlatSink::stream("bar"), Step_FlatSink::end() },
          "foo,bar" },
    };

    void
    flatsink_test_fn(const TestCase_FlatSink & tc,
                     UtestRehearser * p_rh)
    {
        /* 1. flat rendering */
        std::ostringstream ss;
        {
            FlatSink sink(ss.rdbuf());
            flatsink_drive(sink, tc.step_v_);
        }

        REHEARSE(*p_rh, ss.str() == tc.exp_output_);

        /* 2. same steps through PrettySink.  Default soft_right_margin (135)
         *    exceeds every string here, so all groups fit and pretty == flat.
         */

        PrettySink pp(PpConfig::scratch_plain(135), nullptr /*out*/);

        flatsink_drive(pp, tc.step_v_);

        REHEARSE(*p_rh, pp.output() == tc.exp_output_);
        REHEARSE(*p_rh, pp.output() == ss.str());
    }

    TEST_CASE("FlatSink", "[FlatSink]")
    {
        try_test_array(s_flatsink_testcase_v,
                       &flatsink_test_fn);
    }

    /* when a group does NOT fit, the two sinks must diverge:
     * PrettySink honours the split (newline + indent), FlatSink ignores it.
     */
    TEST_CASE("FlatSink.diverges_when_too_wide", "[FlatSink]")
    {
        vector<Step_FlatSink> step_v = { Step_FlatSink::begin(),
                                Step_FlatSink::put("foo,"),
                                Step_FlatSink::split(),
                                Step_FlatSink::put("bar"),
                                Step_FlatSink::end() };

        std::ostringstream ss;
        {
            FlatSink sink(ss.rdbuf());
            flatsink_drive(sink, step_v);
        }

        PrettySink pp(PpConfig::scratch_plain(4),
                      nullptr /*out*/);

        flatsink_drive(pp, step_v);

        REQUIRE(ss.str() == "foo,bar");         // flat: split ignored
        REQUIRE(pp.output() == "foo,\n  bar");  // pretty: split honoured
    }

    /* chained mixed-type insertion straight to the underlying ostream */
    TEST_CASE("FlatSink.stream_chained", "[FlatSink]")
    {
        std::ostringstream ss;
        FlatSink sink(ss.rdbuf());

        {
            auto ins = sink.stream_open(1);
            ins << 42 << ' ' << -7 << " abc";
        }

        REQUIRE(ss.str() == "42 -7 abc");
    }
}

/* end FlatSink.test.cpp */
