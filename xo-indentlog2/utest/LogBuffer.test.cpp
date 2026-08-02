/** @file LogBuffer.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "LogBuffer.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <xo/testutil/try_test_array.hpp>
#include <catch2/catch.hpp>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace ut {
    using xo::LogBuffer;
    using xo::mm::ArenaConfig;
    using xo::UtestRehearser;
    using std::vector;
    using std::size_t;

    /** Unit test setup.
     *  Test sequence:
     *  write text_ into a fresh LogBuffer via write_span(),
     *  then expect:
     *    - used_span() holds exactly the bytes of text_
     *    - lpos() (column on current line) == exp_lpos_
     **/
    struct TestCase_LogBuffer {
        TestCase_LogBuffer(std::vector<const char *> text_v,
                           size_t exp_used,
                           size_t exp_lpos,
                           size_t exp_viz_lpos)
            : text_v_{text_v},
              exp_used_{exp_used},
              exp_lpos_{exp_lpos},
              exp_viz_lpos_{exp_viz_lpos}
            {}

        /** text written to the buffer **/
        std::vector<const char *> text_v_;
        /** expected number of chars used in buffer **/
        size_t exp_used_ = 0;
        /** expected column (local_ppos - solpos) after writing text_ **/
        size_t exp_lpos_ = 0;
        /** expected visible column
         *  (local_npos - solpos - color_escape_chars) after writing text_
         **/
        size_t exp_viz_lpos_ = 0;
    };

    vector<TestCase_LogBuffer>
    s_logbuffer_testcase_v = {
        /* test case: trivial input */
        TestCase_LogBuffer({""}, 0, 0, 0),
        /* test case: trivial input */
        TestCase_LogBuffer({"", "", "", ""}, 0, 0, 0),
        /* test case: single line, no newline: column advances by #chars */
        TestCase_LogBuffer({"abc", "def", "ghi"}, 9, 9, 9),
        /* test case: two lines: column counts from char after the '\n' */
        TestCase_LogBuffer({"hello\nworld"}, 11, 5, 5),
        /* test case: color escape */
        TestCase_LogBuffer({"\033[0m"}, 4, 4, 0),
        /* test case: color escape */
        TestCase_LogBuffer({"\033[31;10m"}, 8, 8, 0),
        /* test case: color escape */
        TestCase_LogBuffer({"\033[38;5;64m"}, 10, 10, 0),
        /* test case: color escape */
        TestCase_LogBuffer({"\033[38;2;80;80;80m"}, 16, 16, 0),
    };

    void
    logbuffer_test_fn(const TestCase_LogBuffer & tc,
                      UtestRehearser * p_rh)
    {
        ArenaConfig cfg { .name_ = "utest.LogBuffer",
                          .size_ = 64*1024 };

        LogBuffer buf(cfg, p_rh->enable_debug());

        REHEARSE(*p_rh, buf.debug_flag() == p_rh->enable_debug());

        /* concatenation of all writes -- expected buffer contents */
        std::ostringstream expected_ss;

        for (const char * text : tc.text_v_) {
            buf.write_span(LogBuffer::ConstSpan::from_cstr(text));
            expected_ss << text;
        }

        std::string expected = expected_ss.str();

        LogBuffer::Span used = buf.used_span();
        LogBuffer::Span committed = buf.committed_span();
        LogBuffer::Span available = buf.available_span();

        REHEARSE(*p_rh, used.is_subspan_of(committed));
        REHEARSE(*p_rh, available.is_subspan_of(committed));
        REHEARSE(*p_rh, used.hi() == available.lo());
        REHEARSE(*p_rh, available.hi() == committed.hi());

        /* refresh line accountant from buffered content (write_span leaves it dirty) */
        buf._check_update_local_state(used.hi());

        REHEARSE(*p_rh, used.size() == expected.size());
        REHEARSE(*p_rh,
                 std::memcmp(used.lo(), expected.data(), expected.size()) == 0);
        REHEARSE(*p_rh, buf.lpos() == tc.exp_lpos_);
        REHEARSE(*p_rh, buf.viz_lpos() == tc.exp_viz_lpos_);

        /** grow buffer (1byte ask will round up to 1page) **/
        buf.expand_to(buf.committed_span().size() + 1);

        LogBuffer::Span used2 = buf.used_span();
        LogBuffer::Span committed2 = buf.committed_span();
        LogBuffer::Span available2 = buf.available_span();

        REHEARSE(*p_rh, used == used2);
        REHEARSE(*p_rh, committed.size() < committed2.size());
        REHEARSE(*p_rh, committed.is_subspan_of(committed2));
        REHEARSE(*p_rh, used2.hi() == available.lo());
        REHEARSE(*p_rh, available.hi() == committed.hi());

        buf.reset_buffer();

        REHEARSE(*p_rh, buf.used_span().empty());
        REHEARSE(*p_rh, buf.used_span().is_subspan_of(buf.committed_span()));
        REHEARSE(*p_rh, buf.available_span().is_subspan_of(buf.committed_span()));
        REHEARSE(*p_rh, buf.lpos() == 0);
        REHEARSE(*p_rh, buf.viz_lpos() == 0);
    }

    TEST_CASE("LogBuffer", "[LogBuffer]")
    {
        try_test_array(s_logbuffer_testcase_v,
                       &logbuffer_test_fn);
    }

    /** Drain semantics: with a streambuf attached via set_dest_sbuf(), the
     *  not-yet-flushed extent [bpptr_, pptr_) is copied out on flush()/
     *  reset_buffer() and the watermark advances, so nothing is re-emitted.
     *  With no dest attached, LogBuffer is a pure accumulator (existing test).
     **/
    TEST_CASE("LogBuffer-drain", "[LogBuffer]")
    {
        ArenaConfig cfg { .name_ = "utest.LogBuffer.drain", .size_ = 64*1024 };

        auto wr = [](LogBuffer & b, const char * s) {
            b.write_span(LogBuffer::ConstSpan::from_cstr(s));
        };

        SECTION("flush drains written bytes to dest") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "hello");
            buf.flush();

            REQUIRE(oss.str() == "hello");
        }

        SECTION("watermark: successive flushes don't re-emit") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "A");
            buf.flush();
            wr(buf, "B");
            buf.flush();            /* must send only "B", not "AB" again */

            REQUIRE(oss.str() == "AB");
        }

        SECTION("flush with nothing pending is a no-op") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "x");
            buf.flush();
            buf.flush();            /* bpptr_ == pptr_: nothing to send */

            REQUIRE(oss.str() == "x");
        }

        SECTION("reset_buffer flushes pending first, then rewinds") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "hello");       /* no explicit flush */
            buf.reset_buffer();

            REQUIRE(oss.str() == "hello");
            REQUIRE(buf.used_span().empty());
        }

        SECTION("reset_buffer after flush doesn't double-emit") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "x");
            buf.flush();
            buf.reset_buffer();

            REQUIRE(oss.str() == "x");
        }

        SECTION("newline_indent '\\n' is included in drained output") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            /* newline_indent advances pptr_ past the '\n' before the accounting
             * call; the flush spans the whole extent so the '\n' is not lost.
             */
            wr(buf, "a");
            buf.newline_indent(2);
            wr(buf, "b");
            buf.reset_buffer();

            REQUIRE(oss.str() == "a\n  b");
        }

        SECTION("multi-record: dest accumulates across resets") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "rec1");
            buf.reset_buffer();
            REQUIRE(buf.used_span().empty());

            wr(buf, "rec2");
            buf.reset_buffer();

            REQUIRE(oss.str() == "rec1rec2");
        }

        SECTION("nullptr dest: pure buffer, flush is a no-op") {
            LogBuffer buf(cfg, false);      /* no dest attached */

            wr(buf, "hello");
            buf.flush();                    /* no-op, no crash */
            REQUIRE(buf.used_span().size() == 5);

            buf.reset_buffer();             /* rewinds without draining */
            REQUIRE(buf.used_span().empty());
        }

        SECTION("detach mid-stream: post-detach content not drained") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "kept");
            buf.flush();
            buf.set_dest_sbuf(nullptr);     /* detach */
            wr(buf, "dropped");
            buf.reset_buffer();             /* flush is now a no-op */

            REQUIRE(oss.str() == "kept");
        }

        SECTION("drain survives in-place expand_to with watermark past origin") {
            std::ostringstream oss;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            wr(buf, "pre");
            buf.flush();                    /* bpptr_ now past porigin_ */

            std::size_t cap0 = buf.committed_span().size();
            std::string big(cap0 + 1000, 'z');   /* exceeds capacity -> expand_to */

            wr(buf, big.c_str());
            buf.flush();

            REQUIRE(buf.committed_span().size() > cap0);   /* grew in place */
            REQUIRE(oss.str() == "pre" + big);             /* "pre" not re-sent */
        }

        SECTION("per-line reclaim: multi-line record drains all lines, buffer stays bounded") {
            std::ostringstream oss;
            std::ostringstream expect;
            LogBuffer buf(cfg, false);
            buf.set_dest_sbuf(oss.rdbuf());

            /* with a dest attached, newline_indent() drains + reclaims each
             * completed line, so a record of many lines never grows the buffer
             * past its initial extent (total content here >> that extent).
             */
            const std::size_t cap0 = buf.committed_span().size();
            const char * line = "line-content-abcdefghijklmnopqrstuvwxyz";  /* 39 chars */
            const int n = 1000;

            for (int i = 0; i < n; ++i) {
                wr(buf, line);
                expect << line;
                if (i + 1 < n) {
                    buf.newline_indent(2);
                    expect << "\n  ";
                }
            }
            buf.reset_buffer();     /* drains the final line */

            REQUIRE(oss.str() == expect.str());          /* every line reached the sink */
            REQUIRE(buf.committed_span().size() == cap0); /* buffer never had to grow */
        }

        SECTION("no reclaim without a dest: multi-line content accumulates in the buffer") {
            LogBuffer buf(cfg, false);      /* no dest attached */

            /* pure-buffer contract: newline_indent must NOT reclaim, so all
             * lines stay in the buffer for used_span()/output() to read.
             */
            wr(buf, "aaa");
            buf.newline_indent(2);
            wr(buf, "bbb");

            LogBuffer::Span used = buf.used_span();
            REQUIRE(used.size() == 3 + 1 + 2 + 3);       /* "aaa" "\n" "  " "bbb" */
            REQUIRE(std::string(used.lo(), used.hi()) == "aaa\n  bbb");
        }
    }
}

/* end LogBuffer.test.cpp */
