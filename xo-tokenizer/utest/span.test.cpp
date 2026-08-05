/** @file span.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Pins span's std::ranges contract.  The static_asserts prove the concepts
 *  are satisfied;  the runtime cases prove the iterators actually walk the
 *  buffer -- a begin()/end() pair that both returned lo_ would satisfy every
 *  concept here and still be wrong.
 **/

#include "xo/tokenizer/span.hpp"
#include <catch2/catch.hpp>
#include <algorithm>
#include <ranges>
#include <string>

namespace xo {
    using xo::scm::span;

    namespace ut {
        /* concept conformance.  contiguous_range is what
         * xo::pp::hex_view (and any generic range algorithm) constrains on.
         */
        static_assert(std::ranges::range<span<char>>);
        static_assert(std::ranges::sized_range<span<char>>);
        static_assert(std::ranges::contiguous_range<span<char>>);
        static_assert(std::ranges::borrowed_range<span<char>>);

        static_assert(std::contiguous_iterator<std::ranges::iterator_t<span<char>>>);
        static_assert(std::same_as<std::ranges::range_value_t<span<char>>, char>);

        static_assert(std::ranges::contiguous_range<span<const char>>);

        TEST_CASE("scm-span-range-walk", "[tokenizer][span][ranges]")
        {
            char buf[] = "abcde";
            /* -1: exclude the trailing NUL */
            span<char> s(buf, buf + sizeof(buf) - 1);

            REQUIRE(s.size() == 5);

            SECTION("ranged-for visits every element in order") {
                std::string seen;
                for (char c : s)
                    seen.push_back(c);

                REQUIRE(seen == "abcde");
            }

            SECTION("begin/end/data agree with lo/hi") {
                REQUIRE(s.begin() == s.lo());
                REQUIRE(s.end() == s.hi());
                REQUIRE(s.data() == s.lo());
                REQUIRE(s.end() - s.begin() == static_cast<std::ptrdiff_t>(s.size()));
            }

            SECTION("std::ranges algorithms accept a span") {
                REQUIRE(std::ranges::find(s, 'c') == s.begin() + 2);
                REQUIRE(std::ranges::count(s, 'a') == 1);
            }

            SECTION("empty span yields no elements") {
                span<char> e;

                REQUIRE(e.empty());
                REQUIRE(std::ranges::distance(e) == 0);
            }
        }

        TEST_CASE("scm-span-borrowed-range", "[tokenizer][span][ranges]")
        {
            char buf[] = "xyz";

            /* the point of enable_borrowed_range: begin() on an *rvalue* span
             * returns a real iterator, not std::ranges::dangling.  Without the
             * specialization this fails to compile.
             */
            auto make = [&buf]() { return span<char>(buf, buf + 3); };

            auto it = std::ranges::begin(make());

            static_assert(!std::same_as<decltype(it), std::ranges::dangling>);

            REQUIRE(*it == 'x');
            REQUIRE(std::ranges::find(make(), 'z') != std::ranges::end(make()));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end span.test.cpp */
