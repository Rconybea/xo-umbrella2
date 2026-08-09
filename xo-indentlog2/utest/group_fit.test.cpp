/** @file group_fit.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Regression tests for PpState's group-fit computation.
 *
 *  A group's width is established at its k_end token (PpState::end).  It used
 *  to be measured as (scan totals now) - (PRINT totals now), but print totals
 *  say nothing about where the group started -- and printing is deferred until
 *  a group's fate is known, so they lag arbitrarily far behind.  The effect was
 *  that a group's measured width included everything scanned before it even
 *  opened, so the deeper the nesting the more foreign content was counted.
 *
 *  Symptoms, all covered below:
 *    - a record whose flat form fits the margin was broken anyway
 *    - structurally identical siblings rendered differently
 *    - output stopped responding to the margin once everything bottomed out
 *    - tags split ":name" from their value with room to spare
 *
 *  The shape used here is the one that exposed it: an Outer with some scalar
 *  fields plus a vector of Inner, where Inner is itself a pretty_struct.  That
 *  is the shape of xo-alloc's GcStatistics / ObjectStatistics, which is where
 *  this was first noticed.
 **/

#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/PrettyVector.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <algorithm>
#include <string>
#include <vector>

namespace ut {
    namespace {
        struct Inner {
            int scanned_n_ = 0;
            int scanned_z_ = 0;
        };

        struct Outer {
            int total_ = 0;
            int n_gc_ = 0;
            std::vector<Inner> per_type_v_;
        };
    } /*namespace*/
} /*namespace ut*/

namespace xo::pp {
    template <>
    struct Prettifier<ut::Inner> {
        static void print(PpSink & sink, const ut::Inner & x) {
            sink.pretty_struct("Inner",
                               field("scanned_n", x.scanned_n_),
                               field("scanned_z", x.scanned_z_));
        }
    };

    template <>
    struct Prettifier<ut::Outer> {
        static void print(PpSink & sink, const ut::Outer & x) {
            sink.pretty_struct("Outer",
                               field("total", x.total_),
                               field("n_gc", x.n_gc_),
                               field("per_type_v", x.per_type_v_));
        }
    };
} /*namespace xo::pp*/

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::mm::ArenaConfig;

    namespace {
        /** render @p x at right margin @p margin.
         *
         *  NB unique arena name per call: two PrettySinks sharing an
         *  ArenaConfig name interfere, and the symptom is wrong indentation in
         *  whichever runs second -- which looks exactly like a printer bug.
         **/
        template <typename T>
        std::string
        render(std::uint32_t margin, const T & x) {
            static int seq = 0;

            ArenaConfig cfg { .name_ = "utest.group_fit." + std::to_string(++seq),
                              .size_ = 64*1024 };

            PrettySink pps(PpConfig().with_logbuf_config(cfg)
                                     .with_soft_right_margin(margin)
                                     .with_style(PpStyle::plain()),
                           nullptr);

            pps.pp(x);

            return std::string(pps.output());
        }

        /** length of the longest line in @p s **/
        std::size_t
        longest_line(const std::string & s) {
            std::size_t lo = 0, worst = 0;

            for (std::size_t i = 0; i <= s.size(); ++i) {
                if ((i == s.size()) || (s[i] == '\n')) {
                    worst = std::max(worst, i - lo);
                    lo = i + 1;
                }
            }

            return worst;
        }

        Outer sample() {
            Outer o;

            o.total_ = 100;
            o.n_gc_ = 3;
            o.per_type_v_.push_back(Inner{1, 16});
            o.per_type_v_.push_back(Inner{2, 32});

            return o;
        }

        /* the flat rendering, for reference: 110 chars */
        constexpr std::string_view c_flat
            = "<Outer :total 100 :n_gc 3 :per_type_v"
              " [<Inner :scanned_n 1 :scanned_z 16>,<Inner :scanned_n 2 :scanned_z 32>]>";
    } /*namespace*/

    TEST_CASE("group_fit-wide-margin-stays-flat", "[group_fit]") {
        REQUIRE(render(200, sample()) == std::string(c_flat));
        REQUIRE(c_flat.size() == 110);
    }

    TEST_CASE("group_fit-inner-group-that-fits-is-not-broken", "[group_fit]") {
        /* THE regression test.  At margin 80 the outer record (110 flat) must
         * break, but the per_type_v vector is only ~72 chars and starts at
         * column 3, so it fits and must stay on one line.
         *
         * Before the fix the vector's measured width included everything
         * scanned before it, so it "did not fit" and every Inner broke.
         */
        std::string out = render(80, sample());

        REQUIRE(out.find("\n  :total 100") != std::string::npos);
        REQUIRE(out.find("[<Inner :scanned_n 1 :scanned_z 16>,"
                         "<Inner :scanned_n 2 :scanned_z 32>]") != std::string::npos);
    }

    TEST_CASE("group_fit-identical-siblings-render-identically", "[group_fit]") {
        /* the two Inner values differ only in their numbers, so at any margin
         * they must break the same way.  Before the fix the second one broke
         * harder than the first, because more content had been scanned by the
         * time it opened.
         */
        for (std::uint32_t margin : {60u, 30u}) {
            std::string out = render(margin, sample());

            INFO("margin " << margin << "\n" << out);

            /* Each element runs from its "<Inner" to the next '>' -- Inner has
             * no nested group, so the first '>' closes it.  (Taking the LAST
             * '>' would swallow the enclosing "]>" on the second element.)
             */
            auto element_at = [&out](std::size_t from) {
                std::size_t lo = out.find("<Inner", from);
                REQUIRE(lo != std::string::npos);
                std::size_t hi = out.find('>', lo);
                REQUIRE(hi != std::string::npos);
                return out.substr(lo, hi + 1 - lo);
            };

            /* digits differ between the two elements by construction; the
             * layout must not
             */
            auto strip_digits = [](std::string s) {
                std::string r;
                for (char c : s) { if (!std::isdigit((unsigned char)c)) r += c; }
                return r;
            };

            std::string a = element_at(0);
            std::string b = element_at(out.find("<Inner") + 1);

            REQUIRE(strip_digits(a) == strip_digits(b));
        }
    }

    TEST_CASE("group_fit-respects-the-margin", "[group_fit]") {
        /* output must actually narrow as the margin narrows.  Before the fix
         * everything bottomed out: margins 40 and 30 produced identical text.
         */
        std::size_t wide = longest_line(render(80, sample()));
        std::size_t narrow = longest_line(render(30, sample()));

        REQUIRE(wide > narrow);
        REQUIRE(narrow <= 30);
    }

    TEST_CASE("group_fit-tag-keeps-name-with-value", "[group_fit]") {
        /* ":scanned_n 1" is short; it must not be split across lines while
         * there is room.  Before the fix the value landed on its own line.
         */
        std::string out = render(30, sample());

        INFO(out);

        REQUIRE(out.find(":scanned_n 1") != std::string::npos);
        REQUIRE(out.find(":scanned_n\n") == std::string::npos);
    }
} /*namespace ut*/

/* end group_fit.test.cpp */
