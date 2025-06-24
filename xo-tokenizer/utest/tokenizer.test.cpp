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
        /** Two-pass test harness.
         *
         *   First pass - verify test assertions.
         *   Second pass only if first pass failed.
         *   On second pass, enable verbose logging
         **/
        struct rehearser {
            rehearser(std::uint32_t att = 0) : attention_{att} {}

            /* expect at most one iterator to exist per TestRehearser instance **/
            struct iterator {
                explicit iterator(rehearser* parent) : parent_{parent} {}

                iterator& operator++();
                std::uint32_t operator*() { return parent_->attention_; }

                bool operator==(const iterator& ix2) const {
                    return (parent_ == ix2.parent_);
                }

                rehearser* parent_ = nullptr;
                std::uint32_t attention_ = 0;

            };

            bool is_first_pass() const { return attention_ == 0; }
            bool is_second_pass() const { return attention_ == 1; }
            bool enable_debug() const { return is_second_pass(); }

            iterator begin() { return iterator(this); }
            iterator end()   { return iterator(nullptr); }

        public:
            /** pass number: 0 or 1 **/
            std::uint32_t attention_ = 0;
            /** @brief set to true when test starts; false if first pass fails **/
            bool ok_flag_ = true;
        };

        auto rehearser::iterator::operator++() -> iterator&
        {
            if (parent_)
                ++(parent_->attention_);

            if (parent_->ok_flag_ && (parent_->attention_ == 1)) {
                /* skip 2nd pass */
                ++(parent_->attention_);
            }

            if (parent_->attention_ == 2)
                parent_ = nullptr;

            return *this;
        }

        /* use this instead of REQUIRE(expr) in context of a test_rehearser */
#      define REHEARSE(rehearser, expr)                    \
        if (rehearser.is_first_pass()) {                   \
            bool _f = (expr);                              \
            rehearser.ok_flag_ = rehearser.ok_flag_ && _f; \
        } else {                                           \
            REQUIRE(expr);                                 \
        }

        /* note: trivial REQUIRE() call in else branch bc we still want
         *       catch2 to count assertions when verification succeeds
         */
#    define REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr) \
        if (catch_flag) {                                \
            REQUIRE((expr));                             \
        } else {                                         \
            REQUIRE(true);                               \
            ok_flag &= (expr);                           \
        }

#    define REQUIRE_ORFAIL(ok_flag, catch_flag, expr)    \
        REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr);    \
        if (!ok_flag)                                    \
            return ok_flag

        namespace {
            struct testcase_tkz {
                std::string input_;
                bool expect_throw_;
                token expected_tk_;
                bool consume_all_;
            };

            std::vector<testcase_tkz>
            s_testcase_v = {
                /*
                 *
                 *        expect_throw              consume_all
                 *        v                         v
                 */
                {"<",     false, token::leftangle(), true},
                /* possible prefix of >= */
                {">",     false, token::rightangle(), true},
                {"> ",    false, token::rightangle(), false},

                {"(",     false, token::leftparen(), true},
                {")",     false, token::rightparen(), true},

                {"[",     false, token::leftbracket(), true},
                {"]",     false, token::rightbracket(), true},

                {"{",     false, token::leftbrace(), true},
                {" {",    false, token::leftbrace(), true},

                {"\t{",   false, token::leftbrace(), true},
                {"\n{",   false, token::leftbrace(), true},
                {"}",     false, token::rightbrace(), true},

                {"0",     false, token::i64_token("0"), true},
                {"1",     false, token::i64_token("1"), true},
                {"12",    false, token::i64_token("12"), true},
                {"123",   false, token::i64_token("123"), true},
                {"1234",  false, token::i64_token("1234"), true},

                {"0 ",    false, token::i64_token("0"), false},
                {"1 ",    false, token::i64_token("1"), false},
                {"12 ",   false, token::i64_token("12"), false},
                {"123 ",  false, token::i64_token("123"), false},
                {"1234 ", false, token::i64_token("1234"), false},

                {"1<",    false, token::i64_token("1"), false},
                {"1>",    false, token::i64_token("1"), false},
                {"1(",    false, token::i64_token("1"), false},
                {"1)",    false, token::i64_token("1"), false},
                {"1[",    false, token::i64_token("1"), false},
                {"1]",    false, token::i64_token("1"), false},
                {"1{",    false, token::i64_token("1"), false},
                {"1}",    false, token::i64_token("1"), false},
                {"1;",    false, token::i64_token("1"), false},
                {"1:",    false, token::i64_token("1"), false},
                {"1,",    false, token::i64_token("1"), false},

                {".1",    false, token::f64_token(".1"), true},
                {".12",   false, token::f64_token(".12"), true},
                {".123",  false, token::f64_token(".123"), true},

                {"+.1",   false, token::f64_token("+.1"), true},
                {"+.12",  false, token::f64_token("+.12"), true},
                {"+.123", false, token::f64_token("+.123"), true},

                {"-.1",   false, token::f64_token("-.1"), true},
                {"-.12",  false, token::f64_token("-.12"), true},
                {"-.123", false, token::f64_token("-.123"), true},

                {"1.",    false, token::f64_token("1."), true},
                {"1.2",   false, token::f64_token("1.2"), true},
                {"1.23",  false, token::f64_token("1.23"), true},

                {"1e0",   false, token::f64_token("1e0"), true},
                {"1e-1",  false, token::f64_token("1e-1"), true},
                {"1e1",   false, token::f64_token("1e1"), true},
                {"1e+1",  false, token::f64_token("1e+1"), true},

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

                {".", false, token::dot(), true},
                {":", false, token::colon(), true},
                {",", false, token::comma(), true},
                {"=", false, token::singleassign(), true},
                {":=", false, token::assign_token(), true},
                {"->", false, token::yields(), true},

                {"+", false, token::plus_token(), true},
                {"-", false, token::minus_token(), true},
                {"*", false, token::star_token(), true},
                {"/", false, token::slash_token(), true},

                {"symbol", false, token::symbol_token("symbol"), true},
                {"another-symbol", false, token::symbol_token("another-symbol"), true},

                {"type", false, token::type(), true},
                {"def", false, token::def(), true},
                {"lambda", false, token::lambda(), true},
                {"if", false, token::if_token(), true},
                {"let", false, token::let(), true},
                {"in", false, token::in(), true},
                {"end", false, token::end(), true},

            };
        }

        TEST_CASE("tokenizer", "[tokenizer]") {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {

                const testcase_tkz & testcase = s_testcase_v[i_tc];

                rehearser rh;

                for (auto _ : rh) {
                    scope log(XO_DEBUG2(rh.enable_debug(), "tokenizer"));

                    log && log(xtag("i_tc", i_tc), xtag("input", testcase.input_));

                    using tokenizer
                        = xo::scm::tokenizer<char>;

                    tokenizer tkz(rh.enable_debug());
                    tokenizer::span_type
                        in_span(testcase.input_.c_str(),
                                testcase.input_.c_str() + testcase.input_.size());

                    auto sr = tkz.scan2(in_span, true /*eof*/);

                    REHEARSE(rh, sr.get_token().tk_type() == testcase.expected_tk_.tk_type());
                    if (sr.get_token().tk_type() == tokentype::tk_i64)
                    {
                        REHEARSE(rh, !sr.get_token().text().empty());
                        REHEARSE(rh, sr.get_token().i64_value() == testcase.expected_tk_.i64_value());
                    } else if (sr.get_token().tk_type() == tokentype::tk_f64)
                    {
                        REHEARSE(rh, !sr.get_token().text().empty());
                        REHEARSE(rh, sr.get_token().f64_value() == testcase.expected_tk_.f64_value());
                    } else if(sr.get_token().tk_type() == tokentype::tk_string)
                    {
                        /* sr.get_token().text() can be empty, consider input "" */
                        REHEARSE(rh, sr.get_token().text() == testcase.expected_tk_.text());
                    } else if(sr.get_token().tk_type() == tokentype::tk_symbol)
                    {
                        REHEARSE(rh, !sr.get_token().text().empty());
                        REHEARSE(rh, sr.get_token().text() == testcase.expected_tk_.text());
                    } else {
                        REHEARSE(rh, sr.get_token().text().empty());
                    }

                    /* must consume all input for tests we're doing here */
                    if (testcase.consume_all_) {
                        REHEARSE(rh, sr.consumed() == in_span);
                    } else {
                        REHEARSE(rh, sr.consumed() != in_span);
                    }
                }
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
                 }},
#ifdef TODO
                {"a.b",
                 false,
                 {token::symbol_token("a"),
                  token::dot(),
                  token::symbol_token("b")
                 }},
#endif
                {"a,b",
                 false,
                 {token::symbol_token("a"),
                  token::comma(),
                  token::symbol_token("b")
                 }},
                {"a:b",
                 false,
                 {token::symbol_token("a"),
                  token::colon(),
                  token::symbol_token("b")
                 }},
                {"a;b",
                 false,
                 {token::symbol_token("a"),
                  token::semicolon(),
                  token::symbol_token("b")
                 }},
                {"a:=b",
                 false,
                 {token::symbol_token("a"),
                  token::assign_token(),
                  token::symbol_token("b")
                 }},
                {"a=b",
                 false,
                 {token::symbol_token("a"),
                  token::singleassign(),
                  token::symbol_token("b")
                 }},
                {"p->q",
                 false,
                 {token::symbol_token("p"),
                  token::yields(),
                  token::symbol_token("q")
                 }},
                {"a + b",
                 false,
                 {token::symbol_token("a"),
                  token::plus_token(),
                  token::symbol_token("b")
                 }},
                {"a - b",
                 false,
                 {token::symbol_token("a"),
                  token::minus_token(),
                  token::symbol_token("b")
                 }},
                {"a-b",
                 false,
                 {token::symbol_token("a-b"),
                 }},
                {"(apple)",
                 false,
                 {token::leftparen(),
                  token::symbol_token("apple"),
                  token::rightparen()
                 }},
                {"<apple>",
                 false,
                 {token::leftangle(),
                  token::symbol_token("apple"),
                  token::rightangle()
                 }},
            };
        }

        TEST_CASE("tokenizer2", "[tokenizer]") {
            /* this time testing token sequences */

            using tokenizer = xo::scm::tokenizer<char>;

            for (std::size_t i_tc = 0, n_tc = s_testcase2_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase2_tkz & testcase = s_testcase2_v[i_tc];

                rehearser rh;

                for (auto _ : rh) {
                    scope log(XO_DEBUG2(rh.enable_debug(), "tokenizer2"));

                    log && log(xtag("i_tc", i_tc), xtag("input", testcase.input_));

                    tokenizer tkz(rh.enable_debug());

                    tokenizer::span_type
                        in_span(testcase.input_.c_str(),
                                testcase.input_.c_str() + testcase.input_.size());

                    for (int i_tk = 0, n_tk = testcase.expected_tk_v_.size();
                         i_tk < n_tk; ++i_tk)
                    {
                        log && log(xtag("i_tk", i_tk));

                        auto sr = tkz.scan2(in_span, in_span.empty());
                        const auto & tk = sr.get_token();

                        if (tk.is_valid()) {
                            REHEARSE(rh, tk.tk_type() == testcase.expected_tk_v_[i_tk].tk_type());
                        }
                        if (tk.tk_type() == tokentype::tk_i64)
                        {
                            REHEARSE(rh, !tk.text().empty());
                            REHEARSE(rh, tk.i64_value() == testcase.expected_tk_v_[i_tk].i64_value());
                        } else if (tk.tk_type() == tokentype::tk_f64)
                        {
                            REHEARSE(rh, !tk.text().empty());
                            REHEARSE(rh, tk.f64_value() == testcase.expected_tk_v_[i_tk].f64_value());
                        } else if(tk.tk_type() == tokentype::tk_string)
                        {
                            /* tk.text() can be empty, consider input "" */
                            REHEARSE(rh, tk.text() == testcase.expected_tk_v_[i_tk].text());
                        } else if(tk.tk_type() == tokentype::tk_symbol)
                        {
                            REHEARSE(rh, !tk.text().empty());
                            REHEARSE(rh, tk.text() == testcase.expected_tk_v_[i_tk].text());
                        } else {
                            REHEARSE(rh, tk.text().empty());
                        }

                        in_span = in_span.after_prefix(sr.consumed());
                    }
                }
            }
        } /*TEST_CASE(tokenizer2)*/

        namespace {
            using tkz_error_type = xo::scm::tokenizer_error<char>;
            using span_type = xo::scm::span<const char>;

            struct testcase_error {
                std::string input_;
                tkz_error_type expect_error_;
            };

            testcase_error
            make_testcase(const char * input, const char * src_function, const char * error_descr,
                          size_t tk_start, size_t whitespace, size_t error_pos)
            {
                testcase_error retval;
                retval.input_ = input;
                retval.expect_error_ = tkz_error_type(src_function, error_descr,
                                                      span_type::from_string(retval.input_),
                                                      tk_start, whitespace, error_pos);
                return retval;
            }

            std::vector<testcase_error>
            s_testcase3_v = {
                //             012345678
                //             --------v
                make_testcase("123.456ez",
                              "assemble_token",
                              "unexpected character in numeric constant",
                              0, 0, 8),
                //             01
                //             -v
                make_testcase("1-3",
                              "assemble_token",
                              "improperly placed sign indicator",
                              0, 0, 1),
                //             012
                //             --v
                make_testcase("1..2",
                              "assemble_token",
                              "duplicate decimal point in numeric literal",
                              0, 0, 2),
                //             0123456
                //             ------v
                make_testcase("1.23e4e",
                              "assemble_token",
                              "duplicate exponent marker in numeric literal",
                              0, 0, 6),
                // tokenizer sees string ["\"]
                //              0 1 2 3
                //              - - - v
                make_testcase("\"\\\"",
                              "assemble_token",
                              "missing terminating '\"' to complete literal string",
                              //"expect \\ to escape one of n|t|r|\"|\\ in string literal",
                              0, 0, 3),
                // tokenizer sees literal with embedded newline
                //                        1         2         3
                //              01234567890123456789012345678901 2
                //              -------------------------------- v
                make_testcase("\"everything was going fine until\n\"",
                              "scan",
                              "must use \\n or \\r to encode newline/cr in string literal",
                              0, 0, 32),
                // tokenizer sees string ["\]
                //              0 1 2
                //              - - v
                make_testcase("\"\\",
                              "assemble_token",
                              "expecting key following escape character \\",
                              0, 0, 2),
                // tokenizer sees string ["\q"]
                //              0 12
                //              - -v
                make_testcase("\"\\q\"",
                              "assemble_token",
                              "expecting one of n|r|\"|\\ following escape \\",
                              0, 0, 2),
                //
                make_testcase("#",
                              "assemble_token",
                              "illegal input character",
                              0, 0, 0),
            };

            TEST_CASE("tokenizer3", "[tokenizer]") {
                /* testing error handling */

                using tokenizer = xo::scm::tokenizer<char>;

                constexpr bool c_force_debug = true;

                for (std::size_t i_tc = 0, n_tc = s_testcase3_v.size(); i_tc < n_tc; ++i_tc) {
                    const testcase_error & testcase = s_testcase3_v[i_tc];

                    rehearser rh(0);

                    for (auto _ : rh) {
                        scope log(XO_DEBUG2(c_force_debug || rh.enable_debug(), "tokenizer3"));

                        log && log(xtag("pass", _), xtag("ok(-)", rh.ok_flag_));
                        log && log(xtag("i_tc", i_tc), xtag("input", testcase.input_));

                        tokenizer tkz(c_force_debug || rh.enable_debug());

                        auto in_span = tokenizer::span_type::from_string(testcase.input_);

                        auto sr = tkz.scan2(in_span, true /*eof*/);

                        REHEARSE(rh, sr.is_error());

                        if (sr.error().src_function()) {
                            REHEARSE(rh, std::string(sr.error().src_function()) == std::string(testcase.expect_error_.src_function()));
                        }
                        if (sr.error().error_description()) {
                            REHEARSE(rh, std::string(sr.error().error_description()) == std::string(testcase.expect_error_.error_description()));
                        }
                        REHEARSE(rh, sr.error().whitespace() == testcase.expect_error_.whitespace());
                        REHEARSE(rh, sr.error().tk_start() == testcase.expect_error_.tk_start());
                        REHEARSE(rh, sr.error().error_pos() == testcase.expect_error_.error_pos());

                        log && log(xtag("ok(+)", rh.ok_flag_));
                    }
                }
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end tokenizer.test.cpp */
