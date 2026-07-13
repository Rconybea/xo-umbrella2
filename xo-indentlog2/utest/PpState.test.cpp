/** @file PpState.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpState.hpp"
#include "print/PpTokenType.hpp"
#include "LogBuffer.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <xo/testutil/try_test_array.hpp>
#include <catch2/catch.hpp>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

namespace ut {
    using xo::print::PpState;
    using xo::print::PpConfig;
    using xo::print::PpTokenFlags;
    using xo::print::k_nominal;
    using xo::print::k_string;
    using xo::print::k_begin;
    using xo::print::k_split;
    using xo::print::k_end;
    using xo::LogBuffer;
    using xo::mm::MemorySizeInfo;
    using xo::mm::ArenaConfig;
    using xo::UtestRehearser;
    using std::cout;
    using std::endl;
    using std::vector;
    using std::size_t;

    struct PpTokenInput {
        PpTokenInput(PpTokenFlags tk_type, const char * tk_str)
            : tk_type_{tk_type}, tk_str_{tk_str}
            {}

        static PpTokenInput string(const char * tk_str) {
            return PpTokenInput(k_string, tk_str);
        }
        static PpTokenInput begin() { return PpTokenInput(k_begin, nullptr); }
        static PpTokenInput split() { return PpTokenInput(k_split, nullptr); }
        static PpTokenInput end()   { return PpTokenInput(k_end,   nullptr); }

        PpTokenFlags tk_type_ = k_nominal;
        const char * tk_str_ = nullptr;
    };

    /** Unit test setup.
     *  Test sequence:
     *  emit each string in token_v_ to a PpState via put_cstr(),
     *  with output connected to a fresh LogBuffer;
     *  then expect the LogBuffer to hold exactly exp_output_.
     **/
    struct TestCase_PpState {
        TestCase_PpState(std::vector<PpTokenInput> token_v,
                         const char * exp_output,
                         std::uint32_t soft_right_margin = 0)
            : token_v_{token_v},
              exp_output_{exp_output},
              soft_right_margin_{soft_right_margin}
            {}

        /** tokens emitted to the pretty printer via put_cstr **/
        std::vector<PpTokenInput> token_v_;
        /** expected pretty-printer output (LogBuffer contents) **/
        const char * exp_output_ = nullptr;
        /** soft right margin for PpConfig; 0 => use PpConfig default **/
        std::uint32_t soft_right_margin_ = 0;
    };

    vector<TestCase_PpState>
    s_ppstate_testcase_v = {
        /* test case: single top-level token flushes verbatim */
        TestCase_PpState({PpTokenInput::string("foo")},
                         "foo"),

        /* test case: empty group -- establishes size 0, must not block printing */
        TestCase_PpState({PpTokenInput::begin(),
                          PpTokenInput::end()},
                         ""),

        /* test case: group that fits within margin -- split collapses to no-op */
        TestCase_PpState({PpTokenInput::begin(),
                          PpTokenInput::string("foo,"),
                          PpTokenInput::split(),
                          PpTokenInput::string("bar"),
                          PpTokenInput::end()},
                         "foo,bar"),

        /* test case: group too wide for soft_right_margin (=4) -- split becomes
         * newline + indent (indent_width default 2, nesting depth 1 => 2 spaces).
         */
        TestCase_PpState({PpTokenInput::begin(),
                          PpTokenInput::string("foo,"),
                          PpTokenInput::split(),
                          PpTokenInput::string("bar"),
                          PpTokenInput::end()},
                         "foo,\n  bar",
                         4 /*soft_right_margin*/),

        /* test case: function call
         *   foo(apple: 1,
         *      banana: 2,
         *      carrot: 3);
         */
        TestCase_PpState(
            {PpTokenInput::string("foo"),
             PpTokenInput::begin(),
             PpTokenInput::string("(apple: 1,"),
             PpTokenInput::split(),
             PpTokenInput::string(" banana: 2,"),
             PpTokenInput::split(),
             PpTokenInput::string(" carrot: 3)"),
             PpTokenInput::end()},
            "foo(apple: 1,\n   banana: 2,\n   carrot: 3)",
            4 /*soft_right_margin*/),
    };

    void
    ppstate_test_fn(const TestCase_PpState & tc,
                    UtestRehearser * p_rh)
    {
        PpConfig cfg;
        if (tc.soft_right_margin_ > 0)
            cfg = cfg.with_soft_right_margin(tc.soft_right_margin_);

        PpState pps(cfg);

        ArenaConfig buf_cfg { .name_ = "utest.PpState",
                              .size_ = 64*1024 };
        LogBuffer logbuf(buf_cfg, p_rh->enable_debug());

        pps.connect_output(&logbuf);

        for (const PpTokenInput & tki : tc.token_v_) {
            switch (tki.tk_type_) {
            case k_string:
                pps.put_cstr(tki.tk_str_);
                break;
            case k_begin:
                pps.begin();
                break;
            case k_split:
                pps.split();
                break;
            case k_end:
                pps.end();
                break;
            default:
                assert(false);
                break;
            }
        }

        LogBuffer::Span used = logbuf.used_span();

        std::string expected = tc.exp_output_;

        REHEARSE(*p_rh,
                 std::string_view(used.lo(), used.hi()) == expected);
        REHEARSE(*p_rh, used.size() == expected.size());

        {
            MemorySizeInfo tk_buffer_info;
            MemorySizeInfo scan_stack_info;
            MemorySizeInfo print_stack_info;
            bool unexpected = false;

            auto visitor = [&tk_buffer_info,
                            &scan_stack_info,
                            &print_stack_info,
                            &unexpected]
                (const MemorySizeInfo & info)
                {
                    if (info.resource_name_ == "pp-tk-buffer") {
                        tk_buffer_info = info;
                    } else if (info.resource_name_ == "pp-scan-stack") {
                        scan_stack_info = info;
                    } else if (info.resource_name_ == "pp-print-stack") {
                        print_stack_info = info;
                    } else {
                        unexpected = true;
                    }
                };

            pps.visit_pools(visitor);

            REHEARSE(*p_rh, unexpected == false);
            REHEARSE(*p_rh, tk_buffer_info.used_ <= tk_buffer_info.allocated_);
            REHEARSE(*p_rh, tk_buffer_info.allocated_ <= tk_buffer_info.committed_);
            REHEARSE(*p_rh, tk_buffer_info.committed_ <= tk_buffer_info.reserved_);

            REHEARSE(*p_rh, scan_stack_info.used_ <= scan_stack_info.allocated_);
            REHEARSE(*p_rh, scan_stack_info.allocated_ <= scan_stack_info.committed_);
            REHEARSE(*p_rh, scan_stack_info.committed_ <= scan_stack_info.reserved_);

            REHEARSE(*p_rh, print_stack_info.used_ <= print_stack_info.allocated_);
            REHEARSE(*p_rh, print_stack_info.allocated_ <= print_stack_info.committed_);
            REHEARSE(*p_rh, print_stack_info.committed_ <= print_stack_info.reserved_);
        };
    }

    TEST_CASE("PpState", "[PpState]")
    {
        try_test_array(s_ppstate_testcase_v,
                       &ppstate_test_fn);
    }
}

/* end PpState.test.cpp */
