/** @file PrettySink.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettySink.hpp"
#include <xo/testutil/try_test_array.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::mm::ArenaConfig;
    using xo::UtestRehearser;
    using std::string;
    using std::vector;

    /** one action in a PrettySink scenario **/
    struct Step_PrettySink {
        enum Kind {
            /** pp.put() **/
            k_put,
            /** stream_open(); ins << text;  -- inserter dtor commits **/
            k_stream_dtor,
            /** stream_open(); ins.os() << text; ins.finish(); **/
            k_stream_finish,
            k_begin,
            k_split,
            k_end,
        };

        static Step_PrettySink put(string s)           { return Step_PrettySink{k_put, std::move(s)}; }
        static Step_PrettySink stream_dtor(string s)   { return Step_PrettySink{k_stream_dtor, std::move(s)}; }
        static Step_PrettySink stream_finish(string s) { return Step_PrettySink{k_stream_finish, std::move(s)}; }
        static Step_PrettySink begin() { return Step_PrettySink{k_begin, ""}; }
        static Step_PrettySink split() { return Step_PrettySink{k_split, ""}; }
        static Step_PrettySink end()   { return Step_PrettySink{k_end, ""}; }

        Kind kind_ = k_put;
        string text_;
    };

    /** Unit test setup.
     *  Test sequence:
     *  drive step_v_ through a fresh PrettySink,
     *  then expect PrettySink::output() to hold exactly exp_output_.
     **/
    struct TestCase_PrettySink {
        vector<Step_PrettySink> step_v_;
        /** expected pretty-printer output **/
        string exp_output_;
        /** soft right margin for PpConfig; 0 => use PpConfig default **/
        std::uint32_t soft_right_margin_ = 0;
    };

    vector<TestCase_PrettySink>
    s_prettyprinter_testcase_v = {
        /* single streamed value, committed by the inserter dtor */
        { { Step_PrettySink::stream_dtor("hello") }, "hello" },

        /* single streamed value, committed explicitly via finish().
         * dtor then invokes finish() again -- must be a no-op (single commit)
         */
        { { Step_PrettySink::stream_finish("hello") }, "hello" },

        /* two consecutive streamed values through one PrettySink.
         * regression for:
         *   (a) PpSinkInserter::os_ never assigned -> crash on first insert
         *   (b) PpTokenStreambuf::commit() nulling pps_ -> crash on second open
         */
        { { Step_PrettySink::stream_dtor("foo"), Step_PrettySink::stream_dtor("bar") }, "foobar" },

        /* same, committed via explicit finish() */
        { { Step_PrettySink::stream_finish("foo"), Step_PrettySink::stream_finish("bar") }, "foobar" },

        /* mixed finish() and dtor commit */
        { { Step_PrettySink::stream_finish("a"), Step_PrettySink::stream_dtor("b") }, "ab" },

        /* several consecutive values -- exercises PpTokenStreambuf reuse,
         * with non-4-aligned token lengths
         */
        { { Step_PrettySink::stream_dtor("a"), Step_PrettySink::stream_dtor("bb"), Step_PrettySink::stream_dtor("ccc"),
            Step_PrettySink::stream_dtor("dddd"), Step_PrettySink::stream_dtor("eeeee") },
          "abbcccddddeeeee" },

        /* streamed values interleaved with put() */
        { { Step_PrettySink::put("["), Step_PrettySink::stream_dtor("x"), Step_PrettySink::put("]") }, "[x]" },

        /* group that fits within the margin: split collapses to a no-op */
        { { Step_PrettySink::begin(), Step_PrettySink::put("foo,"), Step_PrettySink::split(), Step_PrettySink::put("bar"), Step_PrettySink::end() },
          "foo,bar" },

        /* group too wide for soft_right_margin (=4): split becomes newline+indent
         * (indent_width default 2, nesting depth 1 => 2 spaces).
         * Streamed tokens must participate in the fits decision just like put().
         */
        { { Step_PrettySink::begin(), Step_PrettySink::stream_dtor("foo,"), Step_PrettySink::split(),
            Step_PrettySink::stream_dtor("bar"), Step_PrettySink::end() },
          "foo,\n  bar",
          4 /*soft_right_margin*/ },
    };

    void
    prettyprinter_test_fn(const TestCase_PrettySink & tc,
                          UtestRehearser * p_rh)
    {
        ArenaConfig logbuf_cfg { .name_ = "utest.PrettySink",
                                 .size_ = 256*1024 };

        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);

        if (tc.soft_right_margin_ > 0)
            cfg = cfg.with_soft_right_margin(tc.soft_right_margin_);

        PrettySink pp(cfg, nullptr);

        for (const Step_PrettySink & step : tc.step_v_) {
            switch (step.kind_) {
            case Step_PrettySink::k_put:
                pp.put(step.text_);
                break;
            case Step_PrettySink::k_stream_dtor:
                {
                    auto ins = pp.stream_open(1 /*min_z*/);
                    ins << step.text_;      // operator<< convenience
                }                           // <- inserter dtor commits
                break;
            case Step_PrettySink::k_stream_finish:
                {
                    auto ins = pp.stream_open(1 /*min_z*/);
                    ins.os() << step.text_; // os() form
                    ins.finish();           // explicit; dtor's finish() is a no-op
                }
                break;
            case Step_PrettySink::k_begin:
                pp.begin();
                break;
            case Step_PrettySink::k_split:
                pp.split();
                break;
            case Step_PrettySink::k_end:
                pp.end();
                break;
            }
        }

        REHEARSE(*p_rh, pp.output().size() == tc.exp_output_.size());
        REHEARSE(*p_rh, pp.output() == tc.exp_output_);
    }

    TEST_CASE("PrettySink", "[PrettySink]")
    {
        try_test_array(s_prettyprinter_testcase_v,
                       &prettyprinter_test_fn);
    }

    /* chained operator<< of mixed types through a single inserter:
     * exercises num_put (sputc/xsputn) and a single commit at scope exit.
     */
    TEST_CASE("PrettySink.stream_chained", "[PrettySink]")
    {
        PrettySink pp(PpConfig::colored(), nullptr);

        {
            auto ins = pp.stream_open(1);
            ins << 42 << ' ' << -7 << " abc";
        }

        REQUIRE(pp.output() == "42 -7 abc");
    }
}

/* end PrettySink.test.cpp */
