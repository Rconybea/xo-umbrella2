/** @file X1Collector.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "init_gc.hpp"
#include "ListOps.hpp"
#include "DFloat.hpp"
#include "DInteger.hpp"
#include "DList.hpp"
#include "DArray.hpp"

#include <xo/object2/Float.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/List.hpp>

#include <xo/gc/X1Collector.hpp>
#include <xo/alloc2/Arena.hpp>
//#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>

#include <xo/arena/AllocInfo.hpp>
#include <xo/arena/padding.hpp>

#include <xo/subsys/Subsystem.hpp>

#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>

#include <catch2/catch.hpp>

namespace ut {
    using xo::S_gc_tag;
    using xo::scm::ListOps;
    using xo::scm::DList;
    using xo::scm::DArray;
    using xo::scm::DFloat;
    using xo::scm::DInteger;
    using xo::mm::CollectorTypeRegistry;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::AllocHeader;
    using xo::mm::AllocInfo;
    using xo::mm::AGCObject;
    using xo::mm::X1CollectorConfig;
    using xo::mm::DX1Collector;
    using xo::mm::GCRoot;
    using xo::mm::GCObjectStore;
    using xo::mm::GCStatistics;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::mm::Generation;
    using xo::mm::c_n_role;
    using xo::mm::object_age;
    using xo::mm::Role;
    using xo::mm::padding;
    using xo::facet::obj;
    using xo::facet::with_facet;
    using xo::facet::typeseq;
    using xo::Subsystem;
    using xo::InitEvidence;
    using xo::InitSubsys;
    using xo::scope;
    using xo::xtag;

    namespace {
        struct testcase_x1 {
            testcase_x1(std::size_t gz,
                        std::size_t n_gct,
                        std::size_t t_gct)
                : generation_z_{gz},
                  incr_gc_threshold_{n_gct},
                  full_gc_threshold_{t_gct} {}

            std::size_t generation_z_;
            std::size_t incr_gc_threshold_;
            std::size_t full_gc_threshold_;
        };

        std::vector<testcase_x1>
        s_testcase_v = {
            // n_gct: nursery gc threshold
            // t_gct: tenured gc threshold
            //
            //            gz  n_gct  t_gct
            testcase_x1(8192,  1024,  1024)
        };
    }

    static InitEvidence s_init = (InitSubsys<S_gc_tag>::require());

    TEST_CASE("x1-config", "[gc][x1]")
    {
        // real purpose: ensure s_init survives static linking
        REQUIRE(s_init.evidence());

        Subsystem::initialize_all();

        constexpr bool c_debug_flag = false;
        scope log(XO_DEBUG(c_debug_flag), "x1-config test");

        for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            scope log(XO_DEBUG(false), xtag("i_tc", i_tc));

            const testcase_x1 & tc = s_testcase_v[i_tc];

            X1CollectorConfig cfg{ .name_ = "x1_test",
                .arena_config_ = ArenaConfig{
                    .size_ = tc.generation_z_,
                    .store_header_flag_ = true},
                .object_types_z_ = 16384,
                .gc_trigger_v_{{
                        tc.incr_gc_threshold_,
                        tc.full_gc_threshold_}},
                .debug_flag_ = c_debug_flag,
            };

            REQUIRE(cfg.n_generation_ == 2);
            REQUIRE(cfg.n_survive_threshold_ == 2);
            REQUIRE(cfg.age2gen(object_age{0}) == Generation::g0());
            REQUIRE(cfg.age2gen(object_age{1}) == Generation::g0());
            REQUIRE(cfg.age2gen(object_age{2}) == Generation::g1());
            REQUIRE(cfg.age2gen(object_age{99}) == Generation::g1());

            REQUIRE(cfg.promotion_threshold(Generation::g1()) == cfg.n_survive_threshold_);
        }
    }

    TEST_CASE("x1", "[gc][x1]")
    {
        // real purpose: ensure s_init survives static linking
        REQUIRE(s_init.evidence());

        Subsystem::initialize_all();

        /**
         *  This is a basic Collector test for xo-object2 data types
         **/

        constexpr bool c_debug_flag = false;
        scope log(XO_DEBUG(c_debug_flag), "X1Collector test");

        for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            scope log(XO_DEBUG(false), xtag("i_tc", i_tc));

            try {
                const testcase_x1 & tc = s_testcase_v[i_tc];

                X1CollectorConfig cfg{ .name_ = "x1_test",
                                       .arena_config_ = ArenaConfig{
                                           .size_ = tc.generation_z_,
                                           .store_header_flag_ = true},
                                       .object_types_z_ = 16384,
                                       .gc_trigger_v_{{
                                           tc.incr_gc_threshold_,
                                           tc.full_gc_threshold_}},
                                       .sanitize_flag_ = true,
                                       .debug_flag_ = c_debug_flag };

                DX1Collector gc(cfg);

                {
                    // X1Collector never uses the null ctor here.
                    // but it relies on DArenaVector<GCRoot>,
                    // which requires it for DArenaVector<GCRoot>::resize()
                    //
                    GCRoot null_root;

                    REQUIRE(null_root.root() == nullptr);
                }

                // secondary allocator for reporting
                DArena report_arena(ArenaConfig()
                                    .with_name("x1_test_report_arena")
                                    .with_size(64 * 1024));
                auto report_mm = obj<AAllocator,DArena>(&report_arena);

                DArena error_arena(ArenaConfig()
                                   .with_name("x1_test_error_arena")
                                   .with_size(16 * 1024));
                auto error_mm = obj<AAllocator,DArena>(&error_arena);

                const GCObjectStore * p_gco = nullptr;
                {
                    const DX1Collector & c_gc = gc;
                    p_gco = &(c_gc.gco_store());
                }

                CollectorTypeRegistry::instance()
                    .install_types(obj<ACollector,DX1Collector>(&gc));

                DArena * to_0 = nullptr;

                /* verify configuration */
                {
                    REQUIRE(gc.config().arena_config_.size_ == tc.generation_z_);
                    REQUIRE(gc.config().arena_config_.store_header_flag_ == true);
                    REQUIRE(gc.config().n_generation_ == 2);
                }

                /* verify initial collector state */
                {
                    REQUIRE(gc.name() == "x1_test");

                    const DX1Collector::ObjectTypeTable * otypes = gc.get_object_types();

                    REQUIRE(otypes != nullptr);
                    REQUIRE(otypes->store()->reserved() >= cfg.object_types_z_);
                    REQUIRE(otypes->store()->reserved() < cfg.object_types_z_ + otypes->store()->page_z_);

                    const DX1Collector::RootSet * roots = gc.get_root_set();

                    REQUIRE(roots != nullptr);
                    REQUIRE(roots->store()->reserved() >= cfg.object_roots_z_);
                    REQUIRE(roots->store()->reserved() < cfg.object_roots_z_ + roots->store()->page_z_);

                    const DArena * from_0 = gc.get_space(Role::from_space(), Generation{0});

                    REQUIRE(from_0 != nullptr);
                    REQUIRE(from_0->reserved() >= tc.generation_z_);
                    REQUIRE(from_0->reserved() < tc.generation_z_ + from_0->page_z_);
                    REQUIRE(from_0->reserved() % from_0->page_z_ == 0);
                    REQUIRE(from_0->allocated() == 0);

                    const DArena * from_1 = gc.get_space(Role::from_space(), Generation{1});

                    REQUIRE(from_1 != nullptr);
                    REQUIRE(from_1->reserved() == from_0->reserved());
                    REQUIRE(from_1->allocated() == 0);

                    to_0 = gc.get_space(Role::to_space(), Generation{0});

                    REQUIRE(to_0 != nullptr);
                    REQUIRE(to_0->reserved() == from_0->reserved());
                    REQUIRE(to_0->allocated() == 0);

                    const DArena * to_1 = gc.get_space(Role::to_space(), Generation{1});

                    REQUIRE(to_1 != nullptr);
                    REQUIRE(to_1->reserved() == to_0->reserved());
                    REQUIRE(to_1->allocated() == 0);

                    const DArena * from_2 = gc.get_space(Role::from_space(), Generation{2});

                    REQUIRE(from_2 == nullptr);

                    const DArena * to_2 = gc.get_space(Role::to_space(), Generation{2});

                    REQUIRE(to_2 == nullptr);

                    REQUIRE(gc.reserved()
                            == otypes->store()->reserved() + roots->store()->reserved() + 4 * from_0->reserved());

                    log && log(xtag("from_0", from_0->lo_), xtag("to_0", to_0->lo_));
                }

                /* attempt allocation */
                auto gc_o = with_facet<AAllocator>::mkobj(&gc);
                auto c_o = with_facet<ACollector>::mkobj(&gc);

                /* register object types */
                bool ok = CollectorTypeRegistry::instance().install_types(c_o);

                REQUIRE(ok);

                ok = c_o.is_type_installed(typeseq::id<DFloat>());
                REQUIRE(ok);
                ok = c_o.is_type_installed(typeseq::id<DInteger>());
                REQUIRE(ok);
                ok = c_o.is_type_installed(typeseq::id<DList>());
                REQUIRE(ok);
                ok = c_o.is_type_installed(typeseq::id<DArray>());
                REQUIRE(ok);

                REQUIRE(gc_o.name() == cfg.name_);
                // nothing committed yet, execept object types.
                // committed memory will fit on 1 page
                REQUIRE(gc_o.size() == getpagesize());
                // no-op
                REQUIRE(gc_o.expand(0));
                // committed memory will still fit on 1 page,
                REQUIRE(gc_o.size() == getpagesize());

                // x0_o will be added as gc root.  x0_o_orig will not
                auto x0_o = DFloat::box<AGCObject>(gc_o, 3.1415927);
                auto x0_o_orig = x0_o;
                c_o.add_gc_root(&x0_o);
                REQUIRE(to_0->allocated() == sizeof(AllocHeader) + sizeof(DFloat));

                // n1_o will be added as gc root.  n1_o_orig will not
                auto n1_o = DInteger::box<AGCObject>(gc_o, 42);
                auto n1_o_orig = n1_o;
                c_o.add_gc_root(&n1_o);

                REQUIRE(to_0->allocated() == (sizeof(AllocHeader) + sizeof(DFloat)
                                              + sizeof(AllocHeader) + sizeof(DInteger)));

                // l0_o will be added as gc root.  l0_o_orig will not
                auto l0_o = ListOps::list(gc_o, x0_o);
                auto l0_o_orig = l0_o;
                c_o.add_gc_root(&l0_o);
                REQUIRE(to_0->allocated() == (sizeof(AllocHeader) + sizeof(DFloat)
                                              + sizeof(AllocHeader) + sizeof(DInteger)
                                              + sizeof(AllocHeader) + sizeof(DList)));

                auto to0_commit_z = to_0->committed();
                auto to0_reserved_z = to_0->reserved();

                {
                    {
                        REQUIRE(x0_o.iface() != nullptr);
                        REQUIRE(x0_o.data() != nullptr);
                        REQUIRE(gc.contains(Role::to_space(), x0_o.data()));

                        /* check alloc info for newly-allocated object */
                        AllocInfo info = gc.alloc_info((std::byte *)x0_o.data());
                        auto float_tseq = typeseq::id<DFloat>();
                        auto x0_alloc_z = gc.header2size(info.header());

                        REQUIRE(info.age() == 0);
                        REQUIRE(info.tseq() == float_tseq.seqno());
                        REQUIRE(info.size() >= sizeof(DFloat));
                        REQUIRE(info.size() < sizeof(DFloat) + padding::c_alloc_alignment);

                        REQUIRE(sizeof(DFloat) <= x0_alloc_z);
                        REQUIRE(x0_alloc_z <= sizeof(DFloat) + sizeof(AllocHeader));
                        REQUIRE(0 == gc.header2age(info.header()));

                        REQUIRE(float_tseq.seqno() == gc.header2tseq(info.header()));
                        REQUIRE(false == gc.is_forwarding_header(info.header()));

                        REQUIRE(gc.lookup_type(float_tseq));
                        REQUIRE(gc.lookup_type(float_tseq)->_typeseq() == float_tseq);
                    }

                    {
                        REQUIRE(n1_o.iface() != nullptr);
                        REQUIRE(n1_o.data() != nullptr);
                        REQUIRE(gc.contains(Role::to_space(), n1_o.data()));

                        /* check alloc info for newly-allocated object */
                        AllocInfo info = gc.alloc_info((std::byte *)n1_o.data());
                        auto integer_tseq = typeseq::id<DInteger>();
                        auto n1_alloc_z = gc.header2size(info.header());

                        REQUIRE(info.age() == 0);
                        REQUIRE(info.tseq() == integer_tseq.seqno());
                        REQUIRE(info.size() >= sizeof(DInteger));
                        REQUIRE(info.size() < sizeof(DInteger) + padding::c_alloc_alignment);

                        REQUIRE(sizeof(DInteger) <= n1_alloc_z);
                        REQUIRE(n1_alloc_z <= sizeof(DInteger) + sizeof(AllocHeader));
                        REQUIRE(0 == gc.header2age(info.header()));

                        REQUIRE(integer_tseq.seqno() == gc.header2tseq(info.header()));
                        REQUIRE(false == gc.is_forwarding_header(info.header()));

                        REQUIRE(gc.lookup_type(integer_tseq));
                        REQUIRE(gc.lookup_type(integer_tseq)->_typeseq() == integer_tseq);
                    }

                    {
                        REQUIRE(l0_o.iface() != nullptr);
                        REQUIRE(l0_o.data() != nullptr);
                        REQUIRE(gc.contains(Role::to_space(), l0_o.data()));

                        /* check alloc info for newly-allocated object */
                        AllocInfo info = gc.alloc_info((std::byte *)l0_o.data());
                        auto list_tseq = typeseq::id<DList>();
                        auto l0_alloc_z = gc.header2size(info.header());

                        REQUIRE(info.age() == 0);
                        REQUIRE(info.tseq() == list_tseq.seqno());
                        REQUIRE(info.size() >= sizeof(DList));
                        REQUIRE(info.size() < sizeof(DList) + padding::c_alloc_alignment);

                        REQUIRE(sizeof(DList) <= l0_alloc_z);
                        REQUIRE(l0_alloc_z <= sizeof(DList) + sizeof(AllocHeader));
                        REQUIRE(0 == gc.header2age(info.header()));

                        REQUIRE(list_tseq.seqno() == gc.header2tseq(info.header()));
                        REQUIRE(false == gc.is_forwarding_header(info.header()));

                        REQUIRE(gc.lookup_type(list_tseq));
                        REQUIRE(gc.lookup_type(list_tseq)->_typeseq() == list_tseq);
                    }
                }

                {
                    GCStatistics stats = gc.gc_stats();

                    REQUIRE(stats.n_gc() == 0);
                }

                /* no GC roots, so GC is trivial */
                c_o.request_gc(Generation{1});

                log && log(xtag("l0_o.data()", l0_o.data()));
                log && log(xtag("l0_o.data()->head_.data()", l0_o.data()->head_.data()));
                log && log(xtag("x0_o.data()", x0_o.data()));

                // gcobjectstore is stable
                REQUIRE(&gc.gco_store() == p_gco);

                REQUIRE(gc.runstate().gc_upto() == Generation::sentinel());
                REQUIRE(gc.runstate().is_idle());
                REQUIRE(!gc.runstate().is_running());
                REQUIRE(!gc.runstate().is_verify());

                {
                    REQUIRE(!gc.contains(Role::from_space(), x0_o.data()));
                    REQUIRE(gc.contains(Role::to_space(), x0_o.data()));
                    REQUIRE(x0_o.data()->value() == 3.1415927);

                    // former location of x0_o now in from-space
                    REQUIRE(gc.contains(Role::from_space(), x0_o_orig.data()));
                    REQUIRE(gc.locate_address(x0_o_orig.data()) == -1);
                }

                {
                    REQUIRE(!gc.contains(Role::from_space(), n1_o.data()));
                    REQUIRE(gc.contains(Role::to_space(), n1_o.data()));
                    REQUIRE(n1_o.data()->value() == 42);

                    REQUIRE(gc.contains(Role::from_space(), n1_o_orig.data()));
                    REQUIRE(gc.locate_address(n1_o_orig.data()) == -1);
                }

                {
                    REQUIRE(!gc.contains(Role::from_space(), l0_o.data()));
                    REQUIRE(gc.contains(Role::to_space(), l0_o.data()));
                    REQUIRE(l0_o.data()->is_empty() == false);

                    REQUIRE(gc.contains(Role::from_space(), l0_o_orig.data()));
                    REQUIRE(gc.locate_address(l0_o_orig.data()) == -1);

                    REQUIRE((void*)l0_o.data()->head_.data() == (void*)x0_o.data());
                    REQUIRE((void*)l0_o.data()->rest_ == (void*)DList::_nil());
                }

                {
                    // verify a non-gc-owned address
                    int x = 999;

                    REQUIRE(-1 == gc.locate_address(&x));
                }

                REQUIRE(gc.size_total() == gc.committed());
                REQUIRE(gc.mutation_log_entries() == 0);

                Generation g0 = Generation::g0();
                REQUIRE(c_o.committed(g0, Role::to_space()) == to0_commit_z);
                REQUIRE(c_o.reserved(g0, Role::to_space()) == to0_reserved_z);
                REQUIRE(c_o.locate_address(x0_o.data()) >= 0);
                REQUIRE(c_o.contains(Role::to_space(), x0_o.data()));

                GCStatistics stats = gc.gc_stats();
                REQUIRE(stats.n_gc() == 1);

                {
                    obj<AGCObject> report;
                    bool ok = c_o.report_statistics(report_mm, error_mm, &report);
                    REQUIRE(ok);
                    REQUIRE(report);

                    // TODO: print report, verify output

                    report_mm.clear();
                    error_mm.clear();
                }

                {
                    obj<AGCObject> report;
                    bool ok = c_o.report_object_ages(report_mm, error_mm, &report);
                    REQUIRE(ok);
                    REQUIRE(report);

                    // TODO: print report, verify output

                    report_mm.clear();
                    error_mm.clear();
                }

                {
                    obj<AGCObject> report;
                    bool ok = c_o.report_object_types(report_mm, error_mm, &report);
                    REQUIRE(ok);
                    REQUIRE(report);

                    // TODO: print report, verify output

                    report_mm.clear();
                    error_mm.clear();
                }

                gc_o.clear();
                {
                    REQUIRE(gc_o.allocated() == 0);
                }

            } catch (std::exception & ex) {
                std::cerr << "caught exception: " << ex.what() << std::endl;
                REQUIRE(false);
            }
        }
    }
}

/* end X1Collector.test.cpp */
