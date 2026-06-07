/** @file MockCollector.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "MockCollector.hpp"
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/Arena.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::DInteger;
    using xo::mm::ACollector;
    using xo::mm::DMockCollector;
    using xo::mm::MutationLogConfig;
    using xo::mm::MutationLogStore;
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::GCObjectStore;
    using xo::mm::AGCObject;
    using xo::mm::Generation;
    using xo::mm::Role;
    using xo::mm::X1VerifyStats;
    using xo::mm::AAllocator;
    using xo::mm::ArenaConfig;
    using xo::mm::DArena;
    using xo::facet::obj;

    // Gilding the lily here.
    // The only reason to 'test' MockCollector is to suppress
    // false positives on coverage reports.
    // Don't care about MockCollector itself, but it also shows up
    // in ICollector_Xfer, at least on the osx/clang toolchain

    TEST_CASE("MockCollector-1", "[MockCollector]")
    {
        // need to create a {gcos, mls} pair

        constexpr uint32_t c_space_z = 64*1024;
        constexpr uint32_t c_n_gen = 1;
        constexpr uint32_t c_n_survive = 1;
        X1VerifyStats verify_stats;
        GCObjectStoreConfig gcos_config{ArenaConfig()
            .with_name("gcos-arena-name-notused")
            .with_size(c_space_z)
            .with_store_header_flag(true),
            c_n_gen,
            c_n_survive,
            64*1024 /*object_type_z*/,
            false /*debug_flag*/};
        DArena report_arena{ArenaConfig().with_name("report-arena")
            .with_size(64*1024)
            .with_store_header_flag(true)};
        DArena error_arena{ArenaConfig().with_name("error-arena")
            .with_size(64*1024)
            .with_store_header_flag(true)};
        MutationLogConfig mls_config{c_n_gen,
            1024 /*mlog_z*/,
            false /*mlog_enabled_flag*/,
            false /*debug_flag*/};

        GCObjectStore gcos{gcos_config, &verify_stats};
        MutationLogStore mls{mls_config, &gcos};
        DMockCollector mock(&mls, &gcos);

        auto mockgc = obj<ACollector,DMockCollector>(&mock);
        auto report_mm = obj<AAllocator,DArena>(&report_arena);
        auto error_mm = obj<AAllocator,DArena>(&error_arena);

        REQUIRE(mockgc.reserved(Generation::g0(), Role::to_space()) == c_space_z);

        {
            int dummy;

            REQUIRE(mockgc.locate_address(&dummy) == -1);
            REQUIRE(mockgc.contains(Role::to_space(), &dummy) == false);
        }

        {
            obj<AGCObject> rpt;
            {
                // stub
                REQUIRE(mockgc.report_statistics(report_mm, error_mm, &rpt) == false);
                REQUIRE(report_mm.allocated() == 0);
            }
            {
                mockgc.report_object_types(report_mm, error_mm, &rpt);
                REQUIRE(rpt);
                rpt.reset();
                report_mm.clear();
            }
            {
                mockgc.report_object_ages(report_mm, error_mm, &rpt);
                REQUIRE(rpt);
                rpt.reset();
                report_mm.clear();
            }
        }

        {
            auto g0_from = gcos.from_space(Generation::g0());
            REQUIRE(g0_from);
            auto g0_from_mm = obj<AAllocator,DArena>(g0_from);

            // note: we don't need object types in gcos for this test,
            //       since we're not traversing the object graph

            auto x = DInteger::box(g0_from_mm, 42);
            REQUIRE(x);

            auto x_copy = mockgc.alloc_copy((std::byte*)x.data());
            REQUIRE(x_copy);
        }
    }

} /*namespace ut*/

/* end MockCollector.test.cpp */
