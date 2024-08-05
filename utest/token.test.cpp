/* file token.test.cpp
 *
 * author: Roland Conybeare
 */

#include "xo/tokenizer/token.hpp"
#include <catch2/catch.hpp>
#include <memory>

namespace xo {
    using token = xo::scm::token<char>;
    using xo::scm::tokentype;

    namespace ut {
        struct testcase_i64 {
            std::string text_;
            bool expect_throw_;
            std::int64_t expected_;
        };

        std::vector<testcase_i64> s_testcase_v = {
            {"", true, 0},
            {"0", false, 0},
            {"-", true, 0},
            {"+", true, 0},
            {"-0", false, 0},
            {"+0", false, 0},
            {"1", false, 1},
            {"-1", false, -1},
            {"9", false, 9},
            {"-9", false, -9},
            {"12", false, 12},
            {"+12", false, 12},
            {"-12", false, -12},
            {"99", false, 99},
            {"-99", false, -99},
            {"123x", true, 0},
        };

        TEST_CASE("parse-i64", "[token]") {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                INFO(xtag("i_tc", i_tc));

                auto const & testcase = s_testcase_v[i_tc];

                token tk(tokentype::tk_i64,
                         testcase.text_);

                REQUIRE(tk.tk_type() == tokentype::tk_i64);

                bool throw_flag = false;
                try {
                    std::int64_t x = tk.i64_value();

                    REQUIRE(x == testcase.expected_);
                } catch (std::exception & ex) {
                    throw_flag = true;
                }

                REQUIRE(throw_flag == testcase.expect_throw_);
            }
        }

        TEST_CASE("error-i64", "[token]") {
            token tk(tokentype::tk_i64, "+");

            bool throw_flag = false;

            try {
                tk.i64_value();
            } catch(std::exception & ex) {
                throw_flag = true;
            }

            REQUIRE(throw_flag);
        }

        namespace {
            struct testcase_f64 {
                std::string text_;
                bool expect_throw_;
                double expected_;
            };

            std::vector<testcase_f64> s_testcase_v = {
                {"",     true, 0},
                {"0",    false, 0},
                {"-",    true, 0},
                {"+",    true, 0},
                {"-0",   false, 0},

                {"+0",   false, 0},
                {"1",    false, 1},
                {"-1",   false, -1},
                {"9",    false, 9},
                {"-9",   false, -9},

                {"12",   false, 12},
                {"+12",  false, 12},
                {"-12",  false, -12},
                {"99",   false, 99},
                {"-99",  false, -99},

                {"123x", true, 0},
                {"0.0",  false, 0.0},
                {"0.1",  false, 0.1},
                {"0.12", false, 0.12},
                {"0.123", false, 0.123},

                {"0.1234", false, 0.1234},
                {"0.12345", false, 0.12345},
                {"0.123456", false, 0.123456},
                {"0.1234567", false, 0.1234567},
                {"0.12345678", false, 0.12345678},

                {"0.123456789", false, 0.123456789},
                {"+0.0",  false, 0.0},
                {"+0.1",  false, 0.1},
                {"+0.12", false, 0.12},
                {"+0.123", false, 0.123},

                {"+0.1234", false, 0.1234},
                {"+0.12345", false, 0.12345},
                {"+0.123456", false, 0.123456},
                {"+0.1234567", false, 0.1234567},
                {"+0.12345678", false, 0.12345678},

                {"+0.123456789", false, 0.123456789},
                {"+0.0e0",  false, 0.0},
                {"+0.1e0",  false, 0.1},
                {"+0.12e0", false, 0.12},
                {"+0.123e0", false, 0.123},

                {"+0.1234e0", false, 0.1234},
                {"+0.12345e0", false, 0.12345},
                {"+0.123456e0", false, 0.123456},
                {"+0.1234567e0", false, 0.1234567},
                {"+0.12345678e0", false, 0.12345678},

                {"+0.123456789e0", false, 0.123456789},
                {"+0.0e1",  false, 00.},
                {"+0.1e1",  false, 01.},
                {"+0.12e1", false, 01.2},
                {"+0.123e1", false, 01.23},

                {"+0.1234e1", false, 01.234},
                {"+0.12345e1", false, 01.2345},
                {"+0.123456e1", false, 01.23456},
                {"+0.1234567e1", false, 01.234567},
                {"+0.12345678e1", false, 01.2345678},

                {"+0.123456789e1", false, 01.23456789},
                {"+0.0E1",  false, 00.},
                {"+0.1E1",  false, 01.},
                {"+0.12E1", false, 01.2},
                {"+0.123E1", false, 01.23},

                {"+0.1234E1", false, 01.234},
                {"+0.12345E1", false, 01.2345},
                {"+0.123456E1", false, 01.23456},
                {"+0.1234567E1", false, 01.234567},
                {"+0.12345678E1", false, 01.2345678},

                {"+0.123456789E1", false, 01.23456789},
                {"+0.0e9",  false, 0.0},
                {"+0.1e9",  false, 0.1e9},
                {"+0.12e9", false, 0.12e9},
                {"+0.123e9", false, 0.123e9},

                {"+0.1234e9", false, 0.1234e9},
                {"+0.12345e9", false, 0.12345e9},
                {"+0.123456e9", false, 0.123456e9},
                {"+0.1234567e9", false, 0.1234567e9},
                {"+0.12345678e9", false, 0.12345678e9},

                {"+0.123456789e9", false, 0.123456789e9},
                {"-0.0",  false, -0.0},
                {"-0.1",  false, -0.1},
                {"-0.12", false, -0.12},
                {"-0.123", false, -0.123},

                {"-0.1234", false, -0.1234},
                {"-0.12345", false, -0.12345},
                {"-0.123456", false, -0.123456},
                {"-0.1234567", false, -0.1234567},
                {"-0.12345678", false, -0.12345678},

                {"-0.123456789", false, -0.123456789},
                {"00.",  false, 0.0},
                {"01.",  false, 1.0},
                {"01.2", false, 1.2},
                {"01.23", false, 1.23},

                {"01.234", false, 1.234},
                {"01.2345", false, 1.2345},
                {"01.23456", false, 1.23456},
                {"01.234567", false, 1.234567},
                {"01.2345678", false, 1.2345678},

                {"01.23456789", false, 1.23456789},
                {"0.0",  false, 0.0},
                {"1.2",  false, 1.2},
                {"12.", false, 12.0},
                {"12.3", false, 12.3},

                {"12.34", false, 12.34},
                {"12.345", false, 12.345},
                {"12.3456", false, 12.3456},
                {"12.34567", false, 12.34567},
                {"12.345678", false, 12.345678},

                {"12.3456789", false, 12.3456789},
                {"01.23",  false, 1.23},
                {"12.3",  false, 12.3},
                {"123.", false, 123.0},
                {"123.4", false, 123.4},

                {"123.45", false, 123.45},
                {"123.456", false, 123.456},
                {"123.4567", false, 123.4567},
                {"123.45678", false, 123.45678},
                {"123.456789", false, 123.456789},
            };

            TEST_CASE("parse-f64", "[token]") {
                for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                    auto const & testcase = s_testcase_v[i_tc];

                    INFO(tostr(xtag("i_tc", i_tc),
                               xtag("text", testcase.text_)
                             ));

                    token tk(tokentype::tk_f64,
                             testcase.text_);

                    REQUIRE(tk.tk_type() == tokentype::tk_f64);

                    bool throw_flag = false;
                    std::string ex_msg;

                    try {
                        double x = tk.f64_value();

                        REQUIRE(x == Approx(testcase.expected_).epsilon(1.0e-15));
                    } catch (std::exception & ex) {
                        ex_msg = ex.what();

                        throw_flag = true;
                    }

                    INFO(xtag("ex_msg", ex_msg));

                    REQUIRE(throw_flag == testcase.expect_throw_);
                }
            }
        } /*namespace*/
    } /*namespace ut*/
} /*namespace xo*/

/* end token.test.cpp */
