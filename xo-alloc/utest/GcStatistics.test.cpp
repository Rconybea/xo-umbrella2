/* @file GcStatistics.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/GcStatistics.hpp"
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/hex.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <iostream>
#include <catch2/catch.hpp>

#include <string>
#include <ranges>
#include <xo/ppsink/tag_ostream.hpp>

namespace xo {
    using xo::gc::GcStatistics;
    using xo::gc::GcStatisticsExt;
    using xo::gc::PerGenerationStatistics;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::toppstr;

#ifdef OBSOLETE
    namespace {
        /** render @p x with line breaking, as legacy toppstr2(ppconfig, x) did.
         *
         *  toppstr2 is not ported to ppsink: it is superseded by PrettySink,
         *  which is the sink that actually does line breaking (ppsink's own
         *  tostr uses a FlatSink and never breaks).  PrettySink lives in
         *  xo-indentlog2, hence this utest's dependency on it.
         *
         *  NB the arena name must be unique per call: two PrettySinks sharing
         *  an ArenaConfig name interfere, and the symptom is wrong indentation
         *  in whichever case runs second.
         **/
        template <typename T>
        std::string
        toppstr(const T & x) {
            return toppstr(PpConfig::plain(), x);
        }
    } /*namespace*/
#endif

    namespace ut {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (visible via headers that have not
         * migrated) rather than shadowing it.
         */
        using xo::pp::tostr;
        using xo::pp::xtag;

        TEST_CASE("PerGenerationStatistics", "[alloc][gc]")
        {
            PerGenerationStatistics stats;

            std::string s = tostr(stats);

            //std::cerr << hex_view(s.c_str(), s.c_str() + s.length(), true /*as_text*/) << std::endl;



            REQUIRE(s == "<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>");

        }

        TEST_CASE("GcStatistics", "[alloc][gc]")
        {
            GcStatistics stats;

            std::string s = tostr(stats);

            REQUIRE(s == "<GcStatistics :gen_v [<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>,<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>] :total_allocated 0 :total_promoted_sab 0 :total_promoted 0 :n_mutation 0 :n_logged_mutation 0 :n_xgen_mutation 0 :n_xckp_mutation 0>");

        }

        TEST_CASE("GcStatisticsExt", "[alloc][gc]")
        {
            GcStatisticsExt stats;

            std::string s = tostr(stats);



            REQUIRE(s == "<GcStatisticsExt :gen_v [<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>,<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>] :total_allocated 0 :total_promoted_sab 0 :total_promoted 0 :n_mutation 0 :n_logged_mutation 0 :n_xgen_mutation 0 :n_xckp_mutation 0 :nursery_z 0 :nursery_before_checkpoint_z 0 :nursery_after_checkpoint_z 0 :tenured_z 0>");

        }

        TEST_CASE("GcStatistics-pretty", "[alloc][gc][pretty]")
        {
            std::stringstream ss;
            GcStatistics stats;

            std::string actual = toppstr(PpConfig::plain(), stats);
            std::string expected
                = ("<GcStatistics\n"
                   "  :gen_v\n"
                   "   [<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>,\n"
                   "    <PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>]\n"
                   "  :total_allocated 0\n"
                   "  :total_promoted_sab 0\n"
                   "  :total_promoted 0\n"
                   "  :n_mutation 0\n"
                   "  :n_logged_mutation 0\n"
                   "  :n_xgen_mutation 0\n"
                   "  :n_xckp_mutation 0>");

            if (actual != expected) {
                CHECK(actual == expected);
                CHECK(actual.length() == expected.length());

                auto a_ix = actual.begin();
                auto e_ix = expected.begin();

                std::size_t pos = 0;
                while (a_ix != actual.end() && e_ix != expected.end()) {
                    INFO(xtag("pos", pos));
                    INFO(xtag("matching_prefix", std::string(actual.c_str(), pos)));

                    REQUIRE(*a_ix == *e_ix);

                    ++a_ix;
                    ++e_ix;
                    ++pos;
                }
            }



            REQUIRE(actual == expected);

        }

        TEST_CASE("GcStatisticsExt-pretty", "[alloc][gc][pretty]")
        {
            std::stringstream ss;
            GcStatisticsExt stats;

            std::string actual = toppstr(PpConfig::plain(), stats);
            std::string expected
                = ("<GcStatisticsExt\n"
                   "  :gen_v\n"
                   "   [<PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>,\n"
                   "    <PerGenerationStatistics :used_z 0 :n_gc 0 :new_alloc_z 0 :scanned_z 0 :survive_z 0 :promote_z 0>]\n"
                   "  :total_allocated 0\n"
                   "  :total_promoted_sab 0\n"
                   "  :total_promoted 0\n"
                   "  :n_mutation 0\n"
                   "  :n_logged_mutation 0\n"
                   "  :n_xgen_mutation 0\n"
                   "  :n_xckp_mutation 0\n"
                   "  :nursery_z 0\n"
                   "  :nursery_before_checkpoint_z 0\n"
                   "  :nursery_after_checkpoint_z 0\n"
                   "  :tenured_z 0>");

            if (actual != expected) {
                CHECK(actual == expected);
                CHECK(actual.length() == expected.length());

                auto a_ix = actual.begin();
                auto e_ix = expected.begin();

                std::size_t pos = 0;
                while (a_ix != actual.end() && e_ix != expected.end()) {
                    INFO(xtag("pos", pos));
                    INFO(xtag("matching_prefix", std::string(actual.c_str(), pos)));

                    REQUIRE(*a_ix == *e_ix);

                    ++a_ix;
                    ++e_ix;
                    ++pos;
                }
            }

            REQUIRE(actual == expected);
        }
    }
} /*namespace xo*/

/* GcStatistics.test.cpp */
