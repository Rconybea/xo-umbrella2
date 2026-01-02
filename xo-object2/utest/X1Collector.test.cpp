/** @file X1Collector.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DFloat.hpp"
#include "DList.hpp"

#include "IGCObject_DFloat.hpp"
#include "IGCObject_DList.hpp"

#include <xo/gc/Collector.hpp>
#include <xo/gc/DX1Collector.hpp>
#include <xo/gc/detail/IAllocator_DX1Collector.hpp>
#include <xo/alloc2/AllocInfo.hpp>
#include <xo/alloc2/padding.hpp>

#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::DList;
    using xo::scm::DFloat;
    using xo::mm::AAllocator;
    using xo::mm::AllocInfo;
    using xo::mm::AGCObject;
    using xo::mm::DX1Collector;
    using xo::mm::DArena;
    using xo::mm::CollectorConfig;
    using xo::mm::ArenaConfig;
    using xo::mm::generation;
    using xo::mm::role;
    using xo::mm::padding;
    using xo::facet::with_facet;
    using xo::facet::typeseq;

    namespace {
        struct testcase_x1 {
            testcase_x1(std::size_t nz,
                        std::size_t tz,
                        std::size_t n_gct,
                        std::size_t t_gct)
                : nursery_z_{nz},
                  tenured_z_{tz},
                  incr_gc_threshold_{n_gct},
                  full_gc_threshold_{t_gct} {}

            std::size_t nursery_z_;
            std::size_t tenured_z_;
            std::size_t incr_gc_threshold_;
            std::size_t full_gc_threshold_;
        };

        std::vector<testcase_x1>
        s_testcase_v = {
            // n_gct: nursery gc threshold
            // t_gct: tenured gc threshold
            //
            //            nz    tz  n_gct  t_gct
            testcase_x1(4096, 8192,  1024,  1024)
        };
    }

    TEST_CASE("x1", "[gc][x1]")
    {
        for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            try {
                const testcase_x1 & tc = s_testcase_v[i_tc];

                CollectorConfig cfg{
                    .name_ = "x1_test",
                    .arena_config_ = ArenaConfig{
                        .size_ = tc.tenured_z_,
                        .store_header_flag_ = true},
                    .object_types_z_ = 16384,
                    .gc_trigger_v_{{
                            tc.incr_gc_threshold_,
                            tc.full_gc_threshold_}},
                };

                DX1Collector gc(cfg);

                /* verify initial collector state */
                {
                    REQUIRE(gc.name() == "x1_test");

                    const DArena * otypes = gc.get_object_types();

                    REQUIRE(otypes != nullptr);
                    REQUIRE(otypes->reserved() >= cfg.object_types_z_);
                    REQUIRE(otypes->reserved() < cfg.object_types_z_ + otypes->page_z_);

                    DArena * from_0 = gc.get_space(role::from_space(), generation{0});

                    REQUIRE(from_0 != nullptr);
                    REQUIRE(from_0->reserved() >= tc.tenured_z_);
                    REQUIRE(from_0->reserved() < tc.tenured_z_ + from_0->page_z_);
                    REQUIRE(from_0->reserved() % from_0->page_z_ == 0);

                    DArena * from_1 = gc.get_space(role::from_space(), generation{1});

                    REQUIRE(from_1 != nullptr);
                    REQUIRE(from_1->reserved() == from_0->reserved());

                    DArena * to_0 = gc.get_space(role::to_space(), generation{0});

                    REQUIRE(to_0 != nullptr);
                    REQUIRE(to_0->reserved() == from_0->reserved());

                    DArena * to_1 = gc.get_space(role::to_space(), generation{1});

                    REQUIRE(to_1 != nullptr);
                    REQUIRE(to_1->reserved() == to_0->reserved());

                    DArena * from_2 = gc.get_space(role::from_space(), generation{2});

                    REQUIRE(from_2 == nullptr);

                    DArena * to_2 = gc.get_space(role::to_space(), generation{2});

                    REQUIRE(to_2 == nullptr);

                    REQUIRE(gc.reserved_total()
                            == otypes->reserved() + 4 * from_0->reserved());
                }

                /* attempt allocation */
                auto gc_o = with_facet<AAllocator>::mkobj(&gc);

                DFloat * x0 = DFloat::make(gc_o, 3.1415927);
                auto x0_o = with_facet<AGCObject>::mkobj(x0);

                DList * l0 = DList::list(gc_o, x0_o);
                auto l0_o = with_facet<AGCObject>::mkobj(l0);

                {
                    {
                        REQUIRE(x0_o.iface() != nullptr);
                        REQUIRE(x0_o.data() != nullptr);
                        REQUIRE(gc.contains(role::to_space(), x0_o.data()));

                        /* check alloc info for newly-allocated object */
                        AllocInfo info = gc.alloc_info((std::byte *)x0_o.data());

                        REQUIRE(info.age() == 0);
                        REQUIRE(info.tseq() == typeseq::id<DFloat>().seqno());
                        REQUIRE(info.size() >= sizeof(DFloat));
                        REQUIRE(info.size() < sizeof(DFloat) + padding::c_alloc_alignment);
                    }

                    {
                        REQUIRE(l0_o.iface() != nullptr);
                        REQUIRE(l0_o.data() != nullptr);
                        REQUIRE(gc.contains(role::to_space(), l0_o.data()));

                        AllocInfo info = gc.alloc_info((std::byte *)l0_o.data());

                        REQUIRE(info.age() == 0);
                        REQUIRE(info.tseq() == typeseq::id<DList>().seqno());
                        REQUIRE(info.size() >= sizeof(DList));
                        REQUIRE(info.size() < sizeof(DList) + padding::c_alloc_alignment);

                    }
                }

            } catch (std::exception & ex) {
                std::cerr << "caught exception: " << ex.what() << std::endl;
                REQUIRE(false);
            }
        }
    }
}

/* end X1Collector.test.cpp */
