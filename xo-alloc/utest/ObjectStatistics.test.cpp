/* @file ObjectStatistics.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/ObjectStatistics.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/hex.hpp>
#include <xo/ppsink/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <iostream>
#include <catch2/catch.hpp>

#include <string>
#include <xo/ppsink/tag_ostream.hpp>

namespace xo {
    using xo::gc::ObjectStatistics;
    using xo::gc::PerObjectTypeStatistics;
    using xo::reflect::Reflect;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;

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
            auto pps = PrettySink::scratch("utest.alloc.pretty.",
                                           64*1024,
                                           135 /*soft_right_margin*/);

            pps.pp(x);

            return std::string(pps.output());
        }
    } /*namespace*/

    namespace ut {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (visible via headers that have not
         * migrated) rather than shadowing it.
         */
        using xo::pp::tostr;
        using xo::pp::xtag;

        TEST_CASE("PerObjectTypeStatistics", "[alloc][gc]")
        {
            PerObjectTypeStatistics stats;

            std::string s = tostr(stats);

            //std::cerr << hex_view(s.c_str(), s.c_str() + s.length(), true /*as_text*/) << std::endl;



            REQUIRE(s == "<PerObjectTypeStatistics :td nullptr :scanned_n 0 :scanned_z 0 :survive_n 0 :survive_z 0>");

        }

        TEST_CASE("PerObjectTypeStatistics-1", "[alloc][gc]")
        {
            PerObjectTypeStatistics stats;
            stats.td_ = Reflect::require<bool>();
            stats.scanned_n_ = 4;
            stats.scanned_z_ = 16;
            stats.survive_n_ = 2;
            stats.survive_z_ = 8;

            std::string s = tostr(stats);

            //std::cerr << hex_view(s.c_str(), s.c_str() + s.length(), true /*as_text*/) << std::endl;



            REQUIRE(s == "<PerObjectTypeStatistics :td bool :scanned_n 4 :scanned_z 16 :survive_n 2 :survive_z 8>");

        }

        TEST_CASE("ObjectStatistics", "[alloc][gc]")
        {
            ObjectStatistics stats;

            std::string s = tostr(stats);



            REQUIRE(s == "<ObjectTypeStatistics :per_type_stats_v []>");

        }

        TEST_CASE("ObjectStatistics-1", "[alloc][gc]")
        {
            ObjectStatistics stats;
            stats.per_type_stats_v_.push_back(PerObjectTypeStatistics());

            std::string s = tostr(stats);



            REQUIRE(s == "<ObjectTypeStatistics :per_type_stats_v [<PerObjectTypeStatistics :td nullptr :scanned_n 0 :scanned_z 0 :survive_n 0 :survive_z 0>]>");

        }

        TEST_CASE("ObjectStatistics-pretty", "[alloc][gc][pretty]")
        {
            std::stringstream ss;
            ObjectStatistics stats;

            std::string actual = toppstr(stats);
            std::string expected
                = ("<ObjectTypeStatistics :per_type_stats_v []>");

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

        TEST_CASE("ObjectStatistics-pretty-1", "[alloc][gc][pretty]")
        {
            PerObjectTypeStatistics objstats;
            objstats.td_ = Reflect::require<bool>();
            objstats.scanned_n_ = 4;
            objstats.scanned_z_ = 16;
            objstats.survive_n_ = 2;
            objstats.survive_z_ = 8;

            std::stringstream ss;
            ObjectStatistics stats;
            stats.per_type_stats_v_.push_back(objstats);

            std::string actual = toppstr(stats);

            std::string expected
                = ("<ObjectTypeStatistics :per_type_stats_v [<PerObjectTypeStatistics :td bool :scanned_n 4 :scanned_z 16 :survive_n 2 :survive_z 8>]>");

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

        }
    }
} /*namespace xo*/

/* ObjectStatistics.test.cpp */
