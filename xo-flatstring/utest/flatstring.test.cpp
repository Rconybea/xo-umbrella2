/** @file flatstring.utest.cpp **/

#include "xo/flatstring/flatstring.hpp"
#include "xo/flatstring/int128_iostream.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/hex.hpp"
#include <catch2/catch.hpp>
#include <type_traits>
//#include <iostream>

namespace xo {
    using namespace std;

    namespace ut {
        template <typename String>
        void
        flatstring_iter_tests(const String & str, const char * text) {
            size_t n = ::strlen(text);

            REQUIRE(str.size() == n);

            /* verify range iteration visits contents in order */
            {
                size_t i = 0;
                for (char ch : str) {
                    INFO(XTAG(i));

                    CHECK(ch == text[i]);

                    ++i;
                }

                REQUIRE(i == n);
            }

            String str_copy;

            REQUIRE(str_copy.capacity() == str.capacity());
            REQUIRE(str_copy.empty());

            /* verify const iteration visits string elements in order */
            {
                str_copy = str;
                REQUIRE(str_copy == str);

                size_t i = 0;

                for (auto ix = str_copy.cbegin(), end_ix = str_copy.cend(); ix != end_ix; ++ix) {
                    INFO(XTAG(i));

                    char ch = *ix;

                    CHECK(ch == text[i]);

                    ++i;
                }

                REQUIRE(i == n);
            }

            /* verify string overwrite through iterator */
            {
                size_t i = 0;

                for (auto ix = str_copy.begin(), end_ix = str_copy.end(); ix != end_ix; ++ix) {
                    INFO(XTAG(i));

                    *ix = ('a' + i);

                    ++i;
                }

                REQUIRE(i == n);

                for (i = 0; i < n; ++i) {
                    CHECK(str_copy[i] == ('a' + i));
                }
            }

            /* verify reverse iteration visits string elements in reverse order */
            {
                str_copy = str;
                REQUIRE(str_copy == str);

                size_t i = 0;

                for (auto ix = str_copy.rbegin(), end_ix = str_copy.rend(); ix != end_ix; ++ix) {
                    INFO(XTAG(i));

                    char ch = *ix;

                    CHECK(ch == text[n-1-i]);

                    ++i;
                }

                REQUIRE(i == n);
            }

            /* verify string overwrite through reverse iterator */
            {
                str_copy = str;
                REQUIRE(str_copy == str);

                size_t i = 0;

                for (auto ix = str_copy.rbegin(), end_ix = str_copy.rend(); ix != end_ix; ++ix) {
                    INFO(XTAG(i));

                    *ix = ('a' + i);

                    ++i;
                }

                REQUIRE(i == n);

                for (i = 0; i< n; ++i) {
                    CHECK(str_copy[n-1-i] == ('a' + i));
                }
            }

            /* verify const reverse iteration visits string elements in reverse order */
            {
                str_copy = str;
                REQUIRE(str_copy == str);

                size_t i = 0;

                for (auto ix = str_copy.crbegin(), end_ix = str_copy.crend(); ix != end_ix; ++ix) {
                    INFO(XTAG(i));

                    char ch = *ix;

                    CHECK(ch == text[n-1-i]);

                    ++i;
                }

                REQUIRE(i == n);
            }
        }

        template <typename String1, typename String2>
        void
        flatstring_assign_tests(const String1 & str, const char * text,
                                const String2 & str2, const char * text2) {
            INFO(tostr(XTAG(str), XTAG(text), XTAG(text2)));

            String1 str_copy;

            str_copy.assign(str.c_str());
            REQUIRE(str_copy == str);

            /* verify assignment from C-style string **/
            {
                str_copy.assign(text2);

                INFO(tostr(XTAG(str_copy), XTAG(text2)));

                REQUIRE(::strncmp(str_copy.c_str(), text2,
                                  std::min(::strlen(text2)+1, str_copy.capacity())) == 0);
            }

            /* verify assignment from prefix of C-style string */
            for (size_t prefix = 0, n_prefix = ::strlen(text2); prefix < n_prefix; ++prefix)
            {
                str_copy.assign(str);

                REQUIRE(str_copy == str);

                str_copy.assign(text2, prefix);

                INFO(tostr(XTAG(prefix), XTAG(str_copy), XTAG(text2)));

                if (prefix == 0) {
                    REQUIRE(str_copy.empty());
                } else {
                    REQUIRE(str_copy.size() == std::min(prefix, str_copy.capacity()));
                    REQUIRE(::strncmp(str_copy.c_str(), text2,
                                      std::min(prefix, str_copy.capacity())) == 0);
                }
            }

            /* verify assignment from substring */
            String2 text2_copy;
            text2_copy.assign(text2);

            INFO(tostr(XTAG(text2_copy)));

            for (size_t i = 0, n = text2_copy.size(); i < n; ++i) {
                /* deliberately letting j extend beyond the end of text2_copy */
                for (size_t j = i; j < n+10; ++j) {
                    INFO(tostr(XTAG(n), XTAG(i), XTAG(j)));

                    str_copy.assign(str);

                    REQUIRE(str_copy == str);

                    str_copy.assign(text2_copy, i, j-i);

                    INFO(tostr(XTAG(str_copy.fixed_capacity), XTAG(str_copy)));

                    REQUIRE(str_copy.size() == std::min(j-i,
                                                        std::min(text2_copy.size()-i,
                                                                 str_copy.capacity())));
                    REQUIRE(::strncmp(str_copy.c_str(), text2_copy.c_str() + i,
                                      std::min(j-i, str_copy.capacity())) == 0);
                }
            }
        }

        template <typename String1, typename String2>
        void
        flatstring_concat_tests(const String1 & str, const char * text,
                                const String2 & str2, const char * text2)
        {
            flatstring<String1::fixed_capacity + String2::fixed_capacity - 1> concat;

            REQUIRE(concat.empty());

            /* forcing concat to occur at runtime */
            {
                concat = flatstring_concat(str, str2);
                auto req_str = string(text) + string(text2);

                REQUIRE(::strcmp(concat.c_str(), req_str.c_str()) == 0);
            }
            {
                concat = flatstring_concat(str2, str);
                auto req_str = string(text2) + string(text);

                REQUIRE(::strcmp(concat.c_str(), req_str.c_str()) == 0);
            }

#ifdef NOT_USING
            {
                auto concat4 = flatstring_concat(str,
                                                 flatstring(text2),
                                                 str,
                                                 flatstring(text2));
                auto req_str = string(text) + string(text2) + string(text) + string(text2);

                REQUIRE(::strcmp(concat4.c_str(), req_str.c_str()) == 0);
            }
#endif

            {
                auto concat4 = flatstring_concat(str, str2, str, str2);
                auto req_str = string(text) + string(text2) + string(text) + string(text2);

                REQUIRE(::strcmp(concat4.c_str(), req_str.c_str()) == 0);
            }
        }

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

            String str2 = str;

            {
                string str3{str.str()};

                REQUIRE(::strcmp(str3.c_str(), str.c_str()) == 0);
            }

            REQUIRE(string_view(str2) == string_view(str));

            {
                auto cmp = (str2 <=> str);
                REQUIRE(cmp == strong_ordering::equal);
            }

            {
                bool cmp = (str2 == str);
                INFO(xtag("cmp", cmp));
                REQUIRE(str2 == str);

                bool cmp2 = (str2 != str);
                REQUIRE(cmp2 != cmp);
            }

            str2.clear();
            REQUIRE(str2.empty());

            str2.assign(100, ' ');
            REQUIRE(str2.size() == str2.capacity());

            /* verify entirely ' ' */
            {
                size_t i = 0;
                for (char ch : str2) {
                    INFO(XTAG(i));

                    CHECK(ch == ' ');

                    ++i;
                }

                REQUIRE(i == str2.size());
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
#      define LITERAL_TEST_BODY(name, name2, text, text2)                 \
        constexpr flatstring name{text};                                  \
        constexpr flatstring name2{text2};                                \
            static_assert(name[0]==text[0]);                              \
            static_assert(name.at(0)==text[0]);                           \
            static_assert(name.empty() == true || name.empty() == false); \
            static_assert(name.capacity() >= 0);                          \
            static_assert(name.begin() != nullptr);                       \
            static_assert(name.end() != nullptr);                         \
            static_assert(name.cbegin() != nullptr);                      \
            static_assert(name.cend() != nullptr);                        \
            static_assert(name.crbegin()._has_pointer());                 \
            static_assert(name.crend()._has_pointer());                   \
            /*static_assert(name.rbegin() != nullptr);*/                  \
            /*static_assert(!name.rend());*/                              \
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
            flatstring_iter_tests(name, text);                            \
            flatstring_assign_tests(name, text, name2, text2);            \
            flatstring_concat_tests(name, text, name2, text2);            \
            static_assert(string_view(name) == string_view(name));        \
        /* end LITERAL_TEST_BODY */


        TEST_CASE("flatstring", "[flatstring][compile-time]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.flatstring"));
            //log && log("(A)", xtag("foo", foo));

            /* mostly compile-time tests here */

            LITERAL_TEST_BODY(s1,  t1,  "h", "abracadabra!");
            LITERAL_TEST_BODY(s2,  t2,  "he", "bracadabra!");
            LITERAL_TEST_BODY(s3,  t3,  "hel", "racadabra!");
            LITERAL_TEST_BODY(s4,  t4,  "hell", "acadabra!");
            LITERAL_TEST_BODY(s5,  t5,  "hello", "cadabra!");
            LITERAL_TEST_BODY(s6,  t6,  "hello,", "adabra!");
            LITERAL_TEST_BODY(s7,  t7,  "hello, ", "dabra!");
            LITERAL_TEST_BODY(s8,  t8,  "hello, w", "abra!");
            LITERAL_TEST_BODY(s9,  t9,  "hello, wo", "bra!");
            LITERAL_TEST_BODY(s10, t10, "hello, wor", "ra!");
            LITERAL_TEST_BODY(s11, t11, "hello, worl", "a!");
            LITERAL_TEST_BODY(s12, t12, "hello, world", "!");
            LITERAL_TEST_BODY(s13, t13, "hello, world!", "");

            static_assert(s1 == s1);

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

            /* concat */
            static_assert(flatstring_concat(s1,t1) == flatstring("habracadabra!"));

            /* clear */
            auto s13_copy = s13;
            s13_copy.clear();

            REQUIRE(s13_copy.empty());

            constexpr auto s13_copy2 = s13;

            static_assert(s13_copy2.size() == s13.size());

            //cerr << "s13=[" << s13 << "] s13_copy2=[" << s13_copy2 << "]" << endl;
            //cerr << xtag("s13", hex_view(s13.c_str(), s13.c_str() + s13.capacity(), true)) << endl;
            //cerr << xtag("s13_copy2", hex_view(s13_copy2.c_str(), s13_copy2.c_str() + s13_copy2.capacity(), true)) << endl;

            REQUIRE(s13_copy2 == s13);

        } /*TEST_CASE(flatstring)*/

        TEST_CASE("flatstring_int128", "[flatstring]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            //scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.flatstring_int128"));
            //log && log("(A)", xtag("foo", foo));

            __int128_t x = 65536UL*65536UL*65536UL*65536UL*65536UL;

            stringstream ss;
            ss << x;
        } /*TEST_CASE(flatstring_int128)*/


    } /*namespace ut*/
} /*namespace xo*/

/** end flatstring.utest.cpp **/
