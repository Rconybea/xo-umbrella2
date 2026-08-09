/** @file PpTokenStreambuf.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpTokenStreambuf.hpp"
#include "LogBuffer.hpp"
#include "print/PpState.hpp"
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
    using xo::pp::PpState;
    using xo::pp::PpConfig;
    using xo::pp::PpLayoutConfig;
    using xo::pp::PpTokenStreambuf;
    using xo::LogBuffer;
    using xo::mm::ArenaConfig;
    using xo::UtestRehearser;
    using std::string;
    using std::vector;
    using std::size_t;

    /** one action in a test scenario **/
    struct Segment {
        enum Kind { k_literal, k_stream, k_begin, k_end };

        /** emit via PpState::put_cstr() **/
        static Segment literal(string s) { return Segment{k_literal, std::move(s)}; }
        /** emit through a PpTokenStreambuf + std::ostream (operator<<) **/
        static Segment stream(string s)  { return Segment{k_stream, std::move(s)}; }
        /** open a group (PpState::begin()) **/
        static Segment begin() { return Segment{k_begin, ""}; }
        /** close a group (PpState::end()) **/
        static Segment end()   { return Segment{k_end, ""}; }

        Kind kind_ = k_literal;
        string text_;
    };

    /** repeating decimal-digit pattern of length @p n,
     *  so any chunk reorder / duplication / drop shows up in the output.
     **/
    static string
    make_pattern(size_t n) {
        string s;
        s.reserve(n);
        for (size_t i = 0; i < n; ++i)
            s.push_back(char('0' + (i % 10)));
        return s;
    }

    struct TestCase_PpTokenStreambuf {
        vector<Segment> seg_v_;
        string exp_output_;
    };

    vector<TestCase_PpTokenStreambuf>
    s_pptokenstreambuf_testcase_v = {
        /* single short streamed value: fits one chunk, no overflow */
        { { Segment::stream("hello") }, "hello" },

        /* empty streamed value: commit with zero chars -> empty output */
        { { Segment::stream("") }, "" },

        /* stream between literals, non-4-aligned length (5):
         * exercises commit_string's padded scan_ix_ retreat.  If it used the
         * unpadded span() end instead of mem_span(), the trailing "]" token
         * would land misaligned and the output would corrupt.
         */
        { { Segment::literal("["), Segment::stream("hello"), Segment::literal("]") },
          "[hello]" },

        /* consecutive streamed values concatenate (adjacent string tokens) */
        { { Segment::stream("foo"), Segment::stream("bar") }, "foobar" },

        /* several non-aligned lengths (1,2,3) back-to-back with literals */
        { { Segment::literal("("),
            Segment::stream("a"), Segment::literal("|"),
            Segment::stream("bb"), Segment::literal("|"),
            Segment::stream("ccc"),
            Segment::literal(")") },
          "(a|bb|ccc)" },

        /* streamed value with an embedded newline flows through verbatim */
        { { Segment::stream("a\nb") }, "a\nb" },

        /* long streamed value: exceeds one chunk on any page size, forcing the
         * overflow -> commit_string -> open_string -> buffer-wrap path.
         */
        { { Segment::stream(make_pattern(40000)) }, make_pattern(40000) },

        /* long streamed value bracketed by literals */
        { { Segment::literal("<"),
            Segment::stream(make_pattern(40000)),
            Segment::literal(">") },
          "<" + make_pattern(40000) + ">" },

        /* long streamed value INSIDE a group: nothing can flush until end(),
         * so the ring genuinely fills and grows/wraps -- exercises the real
         * wrap path + expand_for, unlike the top-level cases which empty-restart.
         */
        { { Segment::begin(),
            Segment::stream(make_pattern(40000)),
            Segment::end() },
          make_pattern(40000) },

        /* same, with a literal prefix so print_ix_ is off 0 before the group */
        { { Segment::literal("prefix "),
            Segment::begin(),
            Segment::stream(make_pattern(40000)),
            Segment::end() },
          "prefix " + make_pattern(40000) },
    };

    void
    pptokenstreambuf_test_fn(const TestCase_PpTokenStreambuf & tc,
                             UtestRehearser * p_rh)
    {
        PpLayoutConfig cfg;
        PpState pps(cfg);

        ArenaConfig buf_cfg { .name_ = "utest.PpTokenStreambuf",
                              .size_ = 256*1024 };
        LogBuffer logbuf(buf_cfg, p_rh->enable_debug());

        pps.connect_output(&logbuf);

        for (const Segment & seg : tc.seg_v_) {
            switch (seg.kind_) {
            case Segment::k_literal:
                pps.put_cstr(seg.text_.c_str());
                break;
            case Segment::k_stream: {
                PpTokenStreambuf sb(&pps);
                sb.open(1 /*min_z*/);
                std::ostream os(&sb);
                os << seg.text_;
                sb.commit();
                break;
            }
            case Segment::k_begin:
                pps.begin();
                break;
            case Segment::k_end:
                pps.end();
                break;
            }
        }

        LogBuffer::Span used = logbuf.used_span();

        REHEARSE(*p_rh, used.size() == tc.exp_output_.size());
        REHEARSE(*p_rh,
                 std::string_view(used.lo(), used.hi()) == tc.exp_output_);
    }

    TEST_CASE("PpTokenStreambuf", "[PpTokenStreambuf]")
    {
        try_test_array(s_pptokenstreambuf_testcase_v,
                       &pptokenstreambuf_test_fn);
    }

    /* real operator<< integration for arithmetic types
     * (num_put drives sputc/xsputn), independent of the string table.
     */
    TEST_CASE("PpTokenStreambuf.numeric", "[PpTokenStreambuf]")
    {
        PpLayoutConfig cfg;
        PpState pps(cfg);

        ArenaConfig buf_cfg { .name_ = "utest.PpTokenStreambuf.numeric",
                              .size_ = 64*1024 };
        LogBuffer logbuf(buf_cfg, false /*debug*/);

        pps.connect_output(&logbuf);

        {
            PpTokenStreambuf sb(&pps);
            sb.open(1);
            std::ostream os(&sb);
            os << 42 << ' ' << -7;
            sb.commit();
        }

        LogBuffer::Span used = logbuf.used_span();

        REQUIRE(std::string_view(used.lo(), used.hi()) == "42 -7");
    }
}

/* end PpTokenStreambuf.test.cpp */
