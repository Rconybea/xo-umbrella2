/** @file flatstring.utest.cpp **/

#include "xo/flatstring/flatstring.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
//#include <iostream>

namespace xo {
    using namespace std;

    namespace ut {
        template <typename String>
        void
        flatstring_runtime_tests(const String & str, const char * text) {
            INFO(tostr(XTAG(str), XTAG(text)));

            REQUIRE(str.fixed_capacity == strlen(text)+1);
            REQUIRE(str.capacity() == strlen(text));
            REQUIRE(str.size() == strlen(text));
            REQUIRE(str.length() == strlen(text));
            REQUIRE(strcmp(str.c_str(), text) == 0);
            REQUIRE(strcmp(str, text) == 0);

            /* verify range iteration visits contents in order */
            {
                size_t i = 0;
                for (char ch : str) {
                    INFO(XTAG(i));

                    CHECK(ch == text[i]);

                    ++i;
                }
            }
        }

        /* using macro here because template argument depends on size of literal C string,
         * and we can't use such a string as a template argument.
         *
         * static_asserts: using these to verify that constexpr methods are being computed
         * at compile time.
         *
         * REQUIRE() calls to do verification that relies on non-constexpr calls such as
         * strlen(), strcmp()
         */
#      define LITERAL_TEST_BODY(name, text)                           \
        constexpr flatstring name{text};                              \
            static_assert(name[0]==text[0]);                              \
            static_assert(name.at(0)==text[0]);                           \
            static_assert(name.empty() == true || name.empty() == false); \
            static_assert(name.capacity() >= 0);                          \
            static_assert(name.begin() != nullptr);                       \
            static_assert(name.end() != nullptr);                         \
            static_assert(name.cbegin() != nullptr);                      \
            static_assert(name.cend() != nullptr);                        \
            static_assert(name.rbegin() != nullptr);                      \
            static_assert(name.rend() != nullptr);                        \
            static_assert(name.crbegin() != nullptr);                     \
            static_assert(name.crend() != nullptr);                       \
            static_assert(name.size() >= 0);                              \
            static_assert(name.c_str() != nullptr);                       \
            static_assert((name <=> name) == 0);                          \
            static_assert(name == name);                                  \
            static_assert(name >= name);                                  \
            static_assert(name <= name);                                  \
            static_assert(!(name != name));                               \
            static_assert(!(name > name));                                \
            static_assert(!(name < name));                                \
            flatstring_runtime_tests(name, text);                         \
            REQUIRE(name.fixed_capacity == strlen(text)+1);               \
            REQUIRE(name.capacity() == strlen(text));                     \
            REQUIRE(name.size() == strlen(text));                         \
            REQUIRE(name.length() == strlen(text));                       \
            REQUIRE(strcmp(name.c_str(), text) == 0);                     \
            REQUIRE(strcmp(name, text) == 0);                             \
            static_assert(string_view(name) == string_view(name));        \



        TEST_CASE("flatstring", "[flatstring][compile-time]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.flatstring"));
            //log && log("(A)", xtag("foo", foo));

            /* mostly compile-time tests here */

            LITERAL_TEST_BODY(s1,  "h");
            LITERAL_TEST_BODY(s2,  "he");
            LITERAL_TEST_BODY(s3,  "hel");
            LITERAL_TEST_BODY(s4,  "hell");
            LITERAL_TEST_BODY(s5,  "hello");
            LITERAL_TEST_BODY(s6,  "hello,");
            LITERAL_TEST_BODY(s7,  "hello, ");
            LITERAL_TEST_BODY(s8,  "hello, w");
            LITERAL_TEST_BODY(s9,  "hello, wo");
            LITERAL_TEST_BODY(s10, "hello, wor");
            LITERAL_TEST_BODY(s11, "hello, worl");
            LITERAL_TEST_BODY(s12, "hello, world");
            LITERAL_TEST_BODY(s13, "hello, world!");

            static_assert(s1 != s2);
            static_assert(s2 != s3);
            static_assert(s3 != s4);
            static_assert(s4 != s5);
            static_assert(s12 != s13);

            static_assert(s1 < s2);
            static_assert(s2 < s3);
            static_assert(s3 < s4);
            static_assert(s4 < s5);
            static_assert(s12 < s13);

            static_assert(s2 > s1);
            static_assert(s3 > s2);
            static_assert(s4 > s3);
            static_assert(s5 > s4);
            static_assert(s13 > s12);
        } /*TEST_CASE(flatstring)*/

    } /*namespace ut*/
} /*namespace xo*/

/** end flatstring.utest.cpp **/
