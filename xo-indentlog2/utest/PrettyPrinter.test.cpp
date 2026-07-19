/** @file PrettyPrinter.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettyPrinter.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <xo/testutil/try_test_array.hpp>
#include <catch2/catch.hpp>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <cstdint>

namespace ut {
    using xo::print::PrettyPrinter;
    using xo::print::PpConfig;
    using xo::mm::ArenaConfig;
    using xo::UtestRehearser;
    using std::string;
    using std::vector;

    /** one action in a PrettyPrinter scenario **/
    struct Step_PrettyPrinter {
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

        static Step_PrettyPrinter put(string s)           { return Step_PrettyPrinter{k_put, std::move(s)}; }
        static Step_PrettyPrinter stream_dtor(string s)   { return Step_PrettyPrinter{k_stream_dtor, std::move(s)}; }
        static Step_PrettyPrinter stream_finish(string s) { return Step_PrettyPrinter{k_stream_finish, std::move(s)}; }
        static Step_PrettyPrinter begin() { return Step_PrettyPrinter{k_begin, ""}; }
        static Step_PrettyPrinter split() { return Step_PrettyPrinter{k_split, ""}; }
        static Step_PrettyPrinter end()   { return Step_PrettyPrinter{k_end, ""}; }

        Kind kind_ = k_put;
        string text_;
    };

    /** Unit test setup.
     *  Test sequence:
     *  drive step_v_ through a fresh PrettyPrinter,
     *  then expect PrettyPrinter::output() to hold exactly exp_output_.
     **/
    struct TestCase_PrettyPrinter {
        vector<Step_PrettyPrinter> step_v_;
        /** expected pretty-printer output **/
        string exp_output_;
        /** soft right margin for PpConfig; 0 => use PpConfig default **/
        std::uint32_t soft_right_margin_ = 0;
    };

    vector<TestCase_PrettyPrinter>
    s_prettyprinter_testcase_v = {
        /* single streamed value, committed by the inserter dtor */
        { { Step_PrettyPrinter::stream_dtor("hello") }, "hello" },

        /* single streamed value, committed explicitly via finish().
         * dtor then invokes finish() again -- must be a no-op (single commit)
         */
        { { Step_PrettyPrinter::stream_finish("hello") }, "hello" },

        /* two consecutive streamed values through one PrettyPrinter.
         * regression for:
         *   (a) PpSinkInserter::os_ never assigned -> crash on first insert
         *   (b) PpTokenStreambuf::commit() nulling pps_ -> crash on second open
         */
        { { Step_PrettyPrinter::stream_dtor("foo"), Step_PrettyPrinter::stream_dtor("bar") }, "foobar" },

        /* same, committed via explicit finish() */
        { { Step_PrettyPrinter::stream_finish("foo"), Step_PrettyPrinter::stream_finish("bar") }, "foobar" },

        /* mixed finish() and dtor commit */
        { { Step_PrettyPrinter::stream_finish("a"), Step_PrettyPrinter::stream_dtor("b") }, "ab" },

        /* several consecutive values -- exercises PpTokenStreambuf reuse,
         * with non-4-aligned token lengths
         */
        { { Step_PrettyPrinter::stream_dtor("a"), Step_PrettyPrinter::stream_dtor("bb"), Step_PrettyPrinter::stream_dtor("ccc"),
            Step_PrettyPrinter::stream_dtor("dddd"), Step_PrettyPrinter::stream_dtor("eeeee") },
          "abbcccddddeeeee" },

        /* streamed values interleaved with put() */
        { { Step_PrettyPrinter::put("["), Step_PrettyPrinter::stream_dtor("x"), Step_PrettyPrinter::put("]") }, "[x]" },

        /* group that fits within the margin: split collapses to a no-op */
        { { Step_PrettyPrinter::begin(), Step_PrettyPrinter::put("foo,"), Step_PrettyPrinter::split(), Step_PrettyPrinter::put("bar"), Step_PrettyPrinter::end() },
          "foo,bar" },

        /* group too wide for soft_right_margin (=4): split becomes newline+indent
         * (indent_width default 2, nesting depth 1 => 2 spaces).
         * Streamed tokens must participate in the fits decision just like put().
         */
        { { Step_PrettyPrinter::begin(), Step_PrettyPrinter::stream_dtor("foo,"), Step_PrettyPrinter::split(),
            Step_PrettyPrinter::stream_dtor("bar"), Step_PrettyPrinter::end() },
          "foo,\n  bar",
          4 /*soft_right_margin*/ },
    };

    void
    prettyprinter_test_fn(const TestCase_PrettyPrinter & tc,
                          UtestRehearser * p_rh)
    {
        ArenaConfig logbuf_cfg { .name_ = "utest.PrettyPrinter",
                                 .size_ = 256*1024 };

        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);

        if (tc.soft_right_margin_ > 0)
            cfg = cfg.with_soft_right_margin(tc.soft_right_margin_);

        PrettyPrinter pp(cfg);

        for (const Step_PrettyPrinter & step : tc.step_v_) {
            switch (step.kind_) {
            case Step_PrettyPrinter::k_put:
                pp.put(step.text_);
                break;
            case Step_PrettyPrinter::k_stream_dtor:
                {
                    auto ins = pp.stream_open(1 /*min_z*/);
                    ins << step.text_;      // operator<< convenience
                }                           // <- inserter dtor commits
                break;
            case Step_PrettyPrinter::k_stream_finish:
                {
                    auto ins = pp.stream_open(1 /*min_z*/);
                    ins.os() << step.text_; // os() form
                    ins.finish();           // explicit; dtor's finish() is a no-op
                }
                break;
            case Step_PrettyPrinter::k_begin:
                pp.begin();
                break;
            case Step_PrettyPrinter::k_split:
                pp.split();
                break;
            case Step_PrettyPrinter::k_end:
                pp.end();
                break;
            }
        }

        REHEARSE(*p_rh, pp.output().size() == tc.exp_output_.size());
        REHEARSE(*p_rh, pp.output() == tc.exp_output_);
    }

    TEST_CASE("PrettyPrinter", "[PrettyPrinter]")
    {
        try_test_array(s_prettyprinter_testcase_v,
                       &prettyprinter_test_fn);
    }

    /* chained operator<< of mixed types through a single inserter:
     * exercises num_put (sputc/xsputn) and a single commit at scope exit.
     */
    TEST_CASE("PrettyPrinter.stream_chained", "[PrettyPrinter]")
    {
        ArenaConfig logbuf_cfg { .name_ = "utest.PrettyPrinter.chained",
                                 .size_ = 64*1024 };

        PrettyPrinter pp(PpConfig().with_logbuf_config(logbuf_cfg));

        {
            auto ins = pp.stream_open(1);
            ins << 42 << ' ' << -7 << " abc";
        }

        REQUIRE(pp.output() == "42 -7 abc");
    }
}

/* end PrettyPrinter.test.cpp */
