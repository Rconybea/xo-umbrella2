/* file tokenizer.test.cpp
 *
 * author: Roland Conybeare
 */

#include "xo/tokenizer/tokenizer.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::tokentype;
    using token = xo::scm::token<char>;
    using xo::scm::span;

    namespace ut {
        namespace {
            struct testcase_tkz {
                std::string input_;
                bool expect_throw_;
                token expected_tk_;
                bool consume_all_;
            };

            std::vector<testcase_tkz>
            s_testcase_v = {
                {"<", false, token::leftangle(), true},
                {">", false, token::rightangle(), true},

                {"(", false, token::leftparen(), true},
                {")", false, token::rightparen(), true},

                {"[", false, token::leftbracket(), true},
                {"]", false, token::rightbracket(), true},

                {"{", false, token::leftbrace(), true},
                {" {", false, token::leftbrace(), true},

                {"\t{", false, token::leftbrace(), true},
                {"\n{", false, token::leftbrace(), true},
                {"}", false, token::rightbrace(), true},

                {"0",  false, token::i64_token("0"), true},
                {"1",  false, token::i64_token("1"), true},
                {"12",  false, token::i64_token("12"), true},
                {"123",  false, token::i64_token("123"), true},
                {"1234",  false, token::i64_token("1234"), true},

                {"0 ", false, token::i64_token("0"), false},
                {"1 ", false, token::i64_token("1"), false},
                {"12 ", false, token::i64_token("12"), false},
                {"123 ", false, token::i64_token("123"), false},
                {"1234 ", false, token::i64_token("1234"), false},

                {"1<", false, token::i64_token("1"), false},
                {"1>", false, token::i64_token("1"), false},
                {"1(", false, token::i64_token("1"), false},
                {"1)", false, token::i64_token("1"), false},
                {"1[", false, token::i64_token("1"), false},
                {"1]", false, token::i64_token("1"), false},
                {"1{", false, token::i64_token("1"), false},
                {"1}", false, token::i64_token("1"), false},
                {"1;", false, token::i64_token("1"), false},
                {"1:", false, token::i64_token("1"), false},
                {"1,", false, token::i64_token("1"), false},

                {".1", false, token::f64_token(".1"), true},
                {".12", false, token::f64_token(".12"), true},
                {".123", false, token::f64_token(".123"), true},

                {"+.1", false, token::f64_token("+.1"), true},
                {"+.12", false, token::f64_token("+.12"), true},
                {"+.123", false, token::f64_token("+.123"), true},

                {"-.1", false, token::f64_token("-.1"), true},
                {"-.12", false, token::f64_token("-.12"), true},
                {"-.123", false, token::f64_token("-.123"), true},

                {"1.", false, token::f64_token("1."), true},
                {"1.2", false, token::f64_token("1.2"), true},
                {"1.23", false, token::f64_token("1.23"), true},

                {"1e0", false, token::f64_token("1e0"), true},
                {"1e-1", false, token::f64_token("1e-1"), true},
                {"1e1", false, token::f64_token("1e1"), true},
                {"1e+1", false, token::f64_token("1e+1"), true},

                {"\"hello\"", false, token::string_token("hello"), true},
                /* tokenizer sees this input:
                 *   "\"hi\", she said"
                 */
                {"\"\\\"hi\\\", she said\"", false, token::string_token("\"hi\", she said"), true},
                /* tokenizer sees this input:
                 *   "look ma, newline ->\n<- "
                 */
                {"\"look ma, newline ->\\n<- \"", false,
                 token::string_token("look ma, newline ->\n<- "), true},
                /* tokenizer sees this input:
                 *   "tab to the right [\t], to the right [\t]"
                 */
                {"\"tab to the right [\\t], to the right [\\t]\"", false,
                 token::string_token("tab to the right [\t], to the right [\t]"), true},

                {":", false, token::colon(), true},
                {":=", false, token::assign_token(), true},

                {"symbol", false, token::symbol_token("symbol"), true},

                {"type", false, token::type(), true},
                {"def", false, token::def(), true},
                {"lambda", false, token::lambda(), true},
                {"if", false, token::if_token(), true},
                {"let", false, token::let(), true},
                {"in", false, token::in(), true},
                {"end", false, token::end(), true},

                {"*", false, token::star_token(), true},
            };
        }

        TEST_CASE("tokenizer", "[tokenizer]") {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase_tkz & testcase = s_testcase_v[i_tc];

                INFO(xtag("input", testcase.input_));
                INFO(xtag("i_tc", i_tc));

                using tokenizer
                    = xo::scm::tokenizer<char>;

                tokenizer tkz;
                tokenizer::span_type
                    in_span(testcase.input_.c_str(),
                            testcase.input_.c_str() + testcase.input_.size());

                auto out = tkz.scan(in_span);

                auto tk = out.first;

                if (tk.is_invalid())
                    tk = tkz.notify_eof();

                REQUIRE(tk.tk_type() == testcase.expected_tk_.tk_type());
                if (tk.tk_type() == tokentype::tk_i64)
                {
                    REQUIRE(!tk.text().empty());
                    REQUIRE(tk.i64_value() == testcase.expected_tk_.i64_value());
                } else if (tk.tk_type() == tokentype::tk_f64)
                {
                    REQUIRE(!tk.text().empty());
                    REQUIRE(tk.f64_value() == testcase.expected_tk_.f64_value());
                } else if(tk.tk_type() == tokentype::tk_string)
                {
                    /* tk.text() can be empty, consider input "" */
                    REQUIRE(tk.text() == testcase.expected_tk_.text());
                } else if(tk.tk_type() == tokentype::tk_symbol)
                {
                    REQUIRE(!tk.text().empty());
                    REQUIRE(tk.text() == testcase.expected_tk_.text());
                } else {
                    REQUIRE(tk.text().empty());
                }

                /* must consume all input for tests we're doing here */
                if (testcase.consume_all_)
                    REQUIRE(out.second == in_span);
                else
                    REQUIRE(out.second != in_span);
            }
        }

        namespace {
            struct testcase2_tkz {
                std::string input_;
                bool expect_throw_;
                std::vector<token> expected_tk_v_;
            };

            std::vector<testcase2_tkz>
            s_testcase2_v = {
                {"def foo : f64 = 3.141;",
                 false,
                 {token::def(),
                  token::symbol_token("foo"),
                  token::colon(),
                  token::symbol_token("f64"),
                  token::singleassign(),
                  token::f64_token("3.141"),
                  token::semicolon()
                 }},
                {"def foo = lambda (x : f64) { def y = x * x; y; }",
                 false,
                 {token::def(),
                  token::symbol_token("foo"),
                  token::singleassign(),
                  token::lambda(),
                  token::leftparen(),
                  token::symbol_token("x"),
                  token::colon(),
                  token::symbol_token("f64"),
                  token::rightparen(),
                  token::leftbrace(),
                  token::def(),
                  token::symbol_token("y"),
                  token::singleassign(),
                  token::symbol_token("x"),
                  token::star_token(),
                  token::symbol_token("x"),
                  token::semicolon(),
                  token::symbol_token("y"),
                  token::semicolon(),
                  token::rightbrace()
                 }}
            };
        }

        TEST_CASE("tokenizer2", "[tokenizer]") {
            for (std::size_t i_tc = 0, n_tc = s_testcase2_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase2_tkz & testcase = s_testcase2_v[i_tc];

                INFO(xtag("input", testcase.input_));
                INFO(xtag("i_tc", i_tc));

                using tokenizer
                    = xo::scm::tokenizer<char>;

                tokenizer tkz;
                tokenizer::span_type
                    in_span(testcase.input_.c_str(),
                            testcase.input_.c_str() + testcase.input_.size());

                for (int i_tk = 0, n_tk = testcase.expected_tk_v_.size();
                     i_tk < n_tk; ++i_tk)
                {
                    INFO(xtag("i_tk", i_tk));

                    auto res = tkz.scan2(in_span, in_span.empty());
                    const auto & tk = res.first;

                    if (tk.is_valid())
                        REQUIRE(tk.tk_type() == testcase.expected_tk_v_[i_tk].tk_type());
                    if (tk.tk_type() == tokentype::tk_i64)
                    {
                        REQUIRE(!tk.text().empty());
                        REQUIRE(tk.i64_value() == testcase.expected_tk_v_[i_tk].i64_value());
                    } else if (tk.tk_type() == tokentype::tk_f64)
                    {
                        REQUIRE(!tk.text().empty());
                        REQUIRE(tk.f64_value() == testcase.expected_tk_v_[i_tk].f64_value());
                    } else if(tk.tk_type() == tokentype::tk_string)
                    {
                        /* tk.text() can be empty, consider input "" */
                        REQUIRE(tk.text() == testcase.expected_tk_v_[i_tk].text());
                    } else if(tk.tk_type() == tokentype::tk_symbol)
                    {
                        REQUIRE(!tk.text().empty());
                        REQUIRE(tk.text() == testcase.expected_tk_v_[i_tk].text());
                    } else {
                        REQUIRE(tk.text().empty());
                    }

                    in_span = in_span.after_prefix(res.second);
                }
            }
        } /*TEST_CASE(tokenizer2)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end tokenizer.test.cpp */
