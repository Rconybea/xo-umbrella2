/* @file reader.test.cpp */

#include "xo/reader/reader.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::reader;

    namespace ut {
        namespace {
            struct test_case {
                const char * text_;
            };

            std::vector<test_case> s_testcase_v = {
                {"def foo : f64 = 3.14159265;"},
                {"def foo : f64 = (3.14159265);"},
                //{"def foo : f64 = 2.0 * 3.14159265;"},
                {"def foo = lambda (x : f64) 3.1415965;"}
            };
        }

        TEST_CASE("reader", "[reader]") {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag), xtag("utest", "reader"));

            for (std::size_t i_tc = 0; i_tc < s_testcase_v.size(); ++i_tc) {
                const test_case & tc = s_testcase_v[i_tc];

                reader rdr;

                scope log(XO_ENTER2(always, c_debug_flag, "reader.testcase"),
                           xtag("i_tc", i_tc));

                rdr.begin_translation_unit();

                try {
                    auto input
                        = reader::span_type::from_cstr(tc.text_);
                    auto rr
                        = rdr.read_expr(input, true /*eof*/);

                    REQUIRE(rr.expr_.get());

                    log && log(xtag("expr", rr.expr_));

                    input = input.after_prefix(rr.rem_);

                    log && log(xtag("post.input", input));

                    REQUIRE(input.empty());
                } catch (std::exception & ex) {
                    log && log(ex.what());

                    INFO(ex.what());

                    REQUIRE(false);
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end reader.test.cpp */
