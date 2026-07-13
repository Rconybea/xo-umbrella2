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
}

/* end LogBuffer.test.cpp */
