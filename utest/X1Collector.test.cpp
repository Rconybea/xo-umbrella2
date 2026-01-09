/** @file X1Collector.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DFloat.hpp"
#include "DList.hpp"
#include "object2_register_types.hpp"

#include "IGCObject_DFloat.hpp"
#include "IGCObject_DList.hpp"

#include <xo/gc/Collector.hpp>
#include <xo/gc/DX1Collector.hpp>

#include <xo/gc/detail/IAllocator_DX1Collector.hpp>
#include <xo/gc/detail/ICollector_DX1Collector.hpp>

#include <xo/arena/AllocInfo.hpp>
#include <xo/arena/padding.hpp>

#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>

#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::object2_register_types;
    using xo::scm::DList;
    using xo::scm::DFloat;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::AllocHeader;
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
    using xo::scope;
    using xo::xtag;

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
        constexpr bool c_debug_flag = false;
        scope log(XO_DEBUG(c_debug_flag));

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
                    .debug_flag_ = c_debug_flag,
                };

                DX1Collector gc(cfg);

                DArena * to_0 = nullptr;

                /* verify initial collector state */
                {
                    REQUIRE(gc.name() == "x1_test");

                    const DArena * otypes = gc.get_object_types();

                    REQUIRE(otypes != nullptr);
                    REQUIRE(otypes->reserved() >= cfg.object_types_z_);
                    REQUIRE(otypes->reserved() < cfg.object_types_z_ + otypes->page_z_);

                    const DArena * roots = gc.get_roots();

                    REQUIRE(roots != nullptr);
                    REQUIRE(roots->reserved() >= cfg.object_roots_z_);
                    REQUIRE(roots->reserved() < cfg.object_roots_z_ + roots->page_z_);

                    const DArena * from_0 = gc.get_space(role::from_space(), generation{0});

                    REQUIRE(from_0 != nullptr);
                    REQUIRE(from_0->reserved() >= tc.tenured_z_);
                    REQUIRE(from_0->reserved() < tc.tenured_z_ + from_0->page_z_);
                    REQUIRE(from_0->reserved() % from_0->page_z_ == 0);
                    REQUIRE(from_0->allocated() == 0);

                    const DArena * from_1 = gc.get_space(role::from_space(), generation{1});

                    REQUIRE(from_1 != nullptr);
                    REQUIRE(from_1->reserved() == from_0->reserved());
                    REQUIRE(from_1->allocated() == 0);

                    to_0 = gc.get_space(role::to_space(), generation{0});

                    REQUIRE(to_0 != nullptr);
                    REQUIRE(to_0->reserved() == from_0->reserved());
                    REQUIRE(to_0->allocated() == 0);

                    const DArena * to_1 = gc.get_space(role::to_space(), generation{1});

                    REQUIRE(to_1 != nullptr);
                    REQUIRE(to_1->reserved() == to_0->reserved());
                    REQUIRE(to_1->allocated() == 0);

                    const DArena * from_2 = gc.get_space(role::from_space(), generation{2});

                    REQUIRE(from_2 == nullptr);

                    const DArena * to_2 = gc.get_space(role::to_space(), generation{2});

                    REQUIRE(to_2 == nullptr);

                    REQUIRE(gc.reserved_total()
                            == otypes->reserved() + roots->reserved() + 4 * from_0->reserved());

                    log && log(xtag("from_0", from_0->lo_), xtag("to_0", to_0->lo_));
                }

                /* attempt allocation */
                auto gc_o = with_facet<AAllocator>::mkobj(&gc);
                auto c_o = with_facet<ACollector>::mkobj(&gc);

                /* register object types */
                bool ok = object2_register_types(c_o);

                REQUIRE(ok);

                ok = c_o.is_type_installed(typeseq::id<DFloat>());
                REQUIRE(ok);
                ok = c_o.is_type_installed(typeseq::id<DList>());
                REQUIRE(ok);

                DFloat * x0 = DFloat::make(gc_o, 3.1415927);
                auto x0_o = with_facet<AGCObject>::mkobj(x0);
                c_o.add_gc_root(&x0_o);
                REQUIRE(to_0->allocated() == sizeof(AllocHeader) + sizeof(DFloat));

                DList * l0 = DList::list(gc_o, x0_o);
                auto l0_o = with_facet<AGCObject>::mkobj(l0);
                c_o.add_gc_root(&l0_o);
                REQUIRE(to_0->allocated() == (sizeof(AllocHeader) + sizeof(DFloat)
                                              + sizeof(AllocHeader) + sizeof(DList)));

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

                        /* check alloc info for newly-allocated object */
                        AllocInfo info = gc.alloc_info((std::byte *)l0_o.data());

                        REQUIRE(info.age() == 0);
                        REQUIRE(info.tseq() == typeseq::id<DList>().seqno());
                        REQUIRE(info.size() >= sizeof(DList));
                        REQUIRE(info.size() < sizeof(DList) + padding::c_alloc_alignment);
                    }
                }

                /* no GC roots, so GC is trivial */
                c_o.request_gc(generation{1});

                log && log(xtag("l0_o.data()", l0_o.data()));
                log && log(xtag("l0_o.data()->head_.data()", l0_o.data()->head_.data()));
                log && log(xtag("x0_o.data()", x0_o.data()));

                REQUIRE(!gc.contains(role::from_space(), x0_o.data()));
                REQUIRE(gc.contains(role::to_space(), x0_o.data()));
                REQUIRE(x0_o.data()->value() == 3.1415927);
                REQUIRE(!gc.contains(role::from_space(), l0_o.data()));
                REQUIRE(gc.contains(role::to_space(), l0_o.data()));
                REQUIRE(l0_o.data()->is_empty() == false);

                REQUIRE((void*)l0_o.data()->head_.data() == (void*)x0_o.data());

            } catch (std::exception & ex) {
                std::cerr << "caught exception: " << ex.what() << std::endl;
                REQUIRE(false);
            }
        }
    }
}

/* end X1Collector.test.cpp */
