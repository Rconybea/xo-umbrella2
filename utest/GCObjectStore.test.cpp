/** @file GCObjectStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include <xo/gc/GCObjectStore.hpp>
#include "MockCollector.hpp"

#include <xo/object2/ListOps.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::ListOps;
    using xo::scm::DList;
    using xo::scm::DInteger;
    using xo::scm::DBoolean;
    using xo::mm::DMockCollector;
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::GCObjectStore;
    using xo::mm::AGCObject;
    using xo::mm::AGCObjectVisitor;
    using xo::mm::Generation;
    using xo::mm::Role;
    using xo::mm::object_age;
    using xo::mm::ArenaConfig;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::AllocInfo;
    using xo::mm::c_max_generation;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using xo::facet::impl_for;
    using xo::rng::xoshiro256ss;
    using xo::rng::random_seed;
    using xo::scope;
    using xo::xtag;
    using xo::tostr;
    using std::size_t;
    using std::uint32_t;

    namespace {
        struct Testcase {
            explicit Testcase(uint32_t n_gen, uint32_t n_survive,
                              size_t gc_z, uint32_t type_z,
                              bool do_type_registration,
                              uint32_t n_test_obj,
                              uint32_t n_test_assign)
                : n_gen_{n_gen},
                  n_survive_{n_survive},
                  gc_size_{gc_z},
                  object_type_z_{type_z},
                  do_type_registration_{do_type_registration},
                  n_test_obj_{n_test_obj},
                  n_test_assign_{n_test_assign}
                {}

            /** number of generations in gco store **/
            uint32_t n_gen_ = 0;
            /** object promotes on surviving this many gc cycles **/
            uint32_t n_survive_ = 0;
            /** size of each generation's half-space, in bytes **/
            size_t gc_size_ = 0;
            /** Storage for object type array, in bytes.
             *  (need to allow 1 pointer per type)
             **/
            uint32_t object_type_z_ = 0;

            /** if true, register types for
             *  gc-aware types used in unit test
             *  (i.e. DBoolean)
             **/
            bool do_type_registration_ = false;
            /** #of cells in random object graph **/
            uint32_t n_test_obj_ = 0;
            /** #of random assignments to attempt (these may create cycles, for example) **/
            uint32_t n_test_assign_ = 0;
        };

        static std::vector<Testcase> s_testcase_v = {
            /** n_gen, n_survive, gc_size, object_type_z, do_type_registration, n_obj **/
            Testcase(2, 4, 16 * 1024, 8 * 128, false, 0, 0),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  1, 0),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  2, 0),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  4, 0),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  8, 4),
            Testcase(2, 4, 16 * 1024, 8 * 128, true, 16, 7),
        };

        /** record capturing some stats for a (randomly created) gc-aware object **/
        struct Recd {
            Recd() = default;
            Recd(obj<AGCObject> value, uint32_t z, typeseq tseq) : gco_{value}, alloc_z_{z}, tseq_{tseq} {}

            // random gc-aware value
            obj<AGCObject> gco_;
            // expected allocation size (lower bound)
            uint32_t alloc_z_ = 0;
            // representation
            typeseq tseq_;
        };

        /** Create two isomorphic random object graphs containing @p n_obj nodes
         *  Using a few basic data types from xo-object2
         *    DBoolean
         *    DList
         *
         *  Generated objects stored in @p *p_gcos.
         *  Individual items pushed to @p *p_v.
         *
         *  Isomorphic copy in @p *p_arena2,
         *  with individual items pushed to @p *p_v2.
         *
         *  For each i in rance the node (*p_v)[i] is isomorphic to (*p_v2)[i]
         *  (*p_v)[i] allocated entirely from @p p_gcos->new_space()
         *  (*p_v2)[i] allocated entirely from @p p_arena2
         **/
        void
        random_object_graph(uint32_t n_obj,
                            uint32_t n_assign,
                            xoshiro256ss * p_rgen,
                            std::vector<Recd> * p_v,
                            GCObjectStore * p_gcos,
                            std::vector<Recd> * p_v2,
                            DArena * p_arena2)
        {
            if (n_obj == 0)
                return;

            for (uint32_t i_obj = 0; i_obj < n_obj; ++i_obj) {
                auto alloc = obj<AAllocator,DArena>(p_gcos->new_space());
                uint32_t sample = (*p_rgen)() % 100;
                // randomly-constructed node in object graph
                obj<AGCObject> xi;
                uint64_t alloc_z;
                typeseq tseq;

                // 2nd allocator for copy of object model
                auto alloc2 = obj<AAllocator,DArena>(p_arena2);
                // isomorphic node destined for arena2
                obj<AGCObject> xi2;

                if (sample < 50) {
                    // create a DBoolean
                    bool value = ((*p_rgen)() % 2 == 0);

                    xi = DBoolean::box(alloc, value);
                    alloc_z = sizeof(DBoolean);
                    tseq = typeseq::id<DBoolean>();

                    xi2 = DBoolean::box(alloc2, value);
                } else {
                    // create a DList cell, with random {car, cdr}

                    obj<AGCObject> car = ListOps::nil();
                    obj<AGCObject,DList> cdr = ListOps::nil();

                    obj<AGCObject> car2 = ListOps::nil();
                    obj<AGCObject,DList> cdr2 = ListOps::nil();

                    auto z = p_v->size();

                    if (z > 0) {
                        // random car
                        {
                            uint32_t i = ((*p_rgen)() % z);
                            car = p_v->at(i).gco_;

                            car2 = p_v2->at(i).gco_;
                        }

                        // random cdr
                        {
                            uint32_t i = ((*p_rgen)() % z);

                            // is v[i] a list cell?
                            {
                                auto tmp = obj<AGCObject,DList>::from(p_v->at(i).gco_);
                                if (tmp)
                                    cdr = tmp;
                            }

                            {
                                auto tmp2 = obj<AGCObject,DList>::from(p_v2->at(i).gco_);
                                if (tmp2)
                                    cdr2 = tmp2;
                            }
                        }
                    }

                    xi = ListOps::cons(alloc, car, cdr);
                    alloc_z = sizeof(DList);
                    tseq = typeseq::id<DList>();

                    xi2 = ListOps::cons(alloc2, car2, cdr2);
                }

                p_v->push_back(Recd(xi, alloc_z, tseq));

                // also save parallel copy
                p_v2->push_back(Recd(xi2, alloc_z, tseq));
            }

            // also make some random modifications,
            // so that it's possible to create cycles.

            for (uint32_t j = 0; j < n_assign; ++j) {
                // choose an object at random
                uint32_t sample = (*p_rgen)() % n_obj;

                assert(sample < p_v->size());

                // is it a list cell?
                auto xj = obj<AGCObject,DList>::from((*p_v)[sample].gco_);
                auto xj2 = obj<AGCObject,DList>::from((*p_v2)[sample].gco_);

                if (xj) {
                    assert(xj2);

                    // flip a coin -- try modifying one of {car, cdr}
                    sample = (*p_rgen)() % 100;

                    if (sample < 50) {
                        // modify head.  skip usual gc write-barrier stuff

                        sample = (*p_rgen)() % n_obj;
                        // rhs could even be xj itself
                        xj->head_ = (*p_v)[sample].gco_;
                        xj2->head_ = (*p_v2)[sample].gco_;
                    } else {
                        // modify rest, maybe.

                        sample = (*p_rgen)() % n_obj;
                        auto rhs = obj<AGCObject,DList>::from((*p_v)[sample].gco_);
                        auto rhs2 = obj<AGCObject,DList>::from((*p_v2)[sample].gco_);

                        if (rhs) {
                            // modify rest.  skip usual gc write-barrier stuff

                            assert(rhs2);

                            xj->rest_ = rhs.data();
                            xj2->rest_ = rhs2.data();
                        }
                    }
                }
            }
        } /*random_object_graph*/
    } /*namespace*/

    TEST_CASE("GCObjectStore-1", "[GCObjectStore]")
    {
        constexpr bool c_debug_flag = true;
        scope log(XO_DEBUG(c_debug_flag), "GCObjectStore test");

        std::uint64_t seed = 12168164826603821466ul;
        //random_seed(&seed);
        log && log(xtag("seed", seed));

        auto rgen = xoshiro256ss(seed);

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            const Testcase & tc = s_testcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

            /** config for each half-space **/
            ArenaConfig arena_config
                = (ArenaConfig()
                   .with_name("arena-name-not-used")
                   .with_size(tc.gc_size_)
                   .with_store_header_flag(true));

            GCObjectStoreConfig gcos_config(arena_config,
                                            tc.n_gen_,
                                            tc.n_survive_,
                                            tc.object_type_z_,
                                            c_debug_flag);

            /** Parallel arena for reference
             *
             *  We will allocate parallel object model in this arena
             *  for reference; then compare with GCObjectStore behavior.
             *
             *  1. arena2 doesn't have any generation layer cake stuff
             *  2. arena2 doesn't have concept of installed types.
             *     It doesn't have or require any builtin ability to traverse an object model
             **/
            DArena arena2 = DArena::map(ArenaConfig().with_name("arena2-reference")
                                        .with_size(tc.gc_size_ * tc.n_gen_)
                                        .with_store_header_flag(true));

            // object type storage will be empty unless we install a type.
            GCObjectStore gcos(gcos_config);

            // scaffold mock collector doing incremental collection
            DMockCollector mock_gc(&gcos, Generation{0});
            auto mock_gc_visitor = mock_gc.ref<AGCObjectVisitor>();

            REQUIRE(gcos.is_type_installed(typeseq::id<DList>()) == false);
            REQUIRE(gcos.is_type_installed(typeseq::id<DBoolean>()) == false);

            Generation g0{0};
            Generation g1{1};
            Generation gn{tc.n_gen_};

            // install gc-aware types that we intend using in unit test
            if (tc.do_type_registration_) {
                {
                    REQUIRE(gcos.install_type(impl_for<AGCObject,DBoolean>()));
                    REQUIRE(gcos.is_type_installed(typeseq::id<DBoolean>()));
                }
                {
                    REQUIRE(gcos.install_type(impl_for<AGCObject,DList>()));
                    REQUIRE(gcos.is_type_installed(typeseq::id<DList>()));
                }
            }

            // verify basic arena partitioning
            {
                REQUIRE(g0 != g1);
                REQUIRE(gcos.new_space());
                REQUIRE(gcos.new_space() == gcos.to_space(g0));
                REQUIRE(gcos.new_space()->reserved() >= tc.gc_size_);
                REQUIRE(gcos.from_space(g0));

                for (Generation gi = g1; gi < tc.n_gen_; ++gi) {
                    // all configured generations exist
                    REQUIRE(gcos.to_space(gi));
                    REQUIRE(gcos.from_space(gi));

                    // to- and from- space are distinct
                    REQUIRE(gcos.to_space(gi) != gcos.from_space(gi));

                    // arenas for different generations are distinct
                    for (Generation gj = g0; gj < gi; ++gj) {
                        REQUIRE(gcos.to_space(gi) != gcos.to_space(gj));
                        REQUIRE(gcos.from_space(gi) != gcos.to_space(gj));

                        REQUIRE(gcos.to_space(gi) != gcos.from_space(gj));
                        REQUIRE(gcos.from_space(gi) != gcos.to_space(gj));
                    }
                }

                // generations that weren't requested, don't exist
                if (gn < c_max_generation) {
                    REQUIRE(!gcos.to_space(gn));
                    REQUIRE(!gcos.from_space(gn));
                }
            }

            // verify we have non-zero space!
            {
                for (Generation gi = g0; gi < gn; ++gi) {
                    INFO(tostr(xtag("gi", gi)));

                    REQUIRE(gcos.to_space(gi)->allocated() == 0);
                    REQUIRE(gcos.to_space(gi)->reserved() >= tc.gc_size_);

                    REQUIRE(gcos.from_space(gi)->allocated() == 0);
                    REQUIRE(gcos.from_space(gi)->reserved() >= tc.gc_size_);
                }
            }

            // allocator
            auto alloc = obj<AAllocator,DArena>(gcos.new_space());

            // create object(s).
            // details depend on test case

            std::vector<Recd> x1_v;
            std::vector<Recd> x2_v;
            {
                random_object_graph(tc.n_test_obj_,
                                    tc.n_test_assign_,
                                    &rgen,
                                    &x1_v,
                                    &gcos,
                                    &x2_v,
                                    &arena2);

                //x1_v.push_back(Recd(DBoolean::box(alloc, true),
                //                    sizeof(DBoolean),
                //                    typeseq::id<DBoolean>()));
            }

            // someday: print the graph. Need a cycle-detecting printer

            REQUIRE(x1_v.size() == x2_v.size());
            for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
                REQUIRE(x1_v[i].alloc_z_ == x2_v[i].alloc_z_);
                REQUIRE(x1_v[i].tseq_ == x2_v[i].tseq_);

                REQUIRE(x1_v[i].gco_._typeseq() == x1_v[i].tseq_);
                REQUIRE(x2_v[i].gco_._typeseq() == x2_v[i].tseq_);
            }

            // gcos can reveal info about allocs
            for (size_t i = 0, n = x1_v.size(); i < n; ++i)
            {
                const auto & x1 = x1_v.at(i);

                REQUIRE(gcos.contains_allocated(Role::to_space(), x1.gco_.data()));
                AllocInfo obj_info = gcos.alloc_info((std::byte *)x1.gco_.data());
                REQUIRE(obj_info.size() >= x1.alloc_z_);

                REQUIRE(obj_info.payload().first == (std::byte *)x1.gco_.data());
                REQUIRE(obj_info.tseq() == x1.tseq_.seqno());

                // also can use header2size / header2tseq convenience functions
                REQUIRE(gcos.header2size(obj_info.header()) == obj_info.size());
                REQUIRE(gcos.header2age(obj_info.header()) == object_age{0});
                REQUIRE(gcos.header2tseq(obj_info.header()) == obj_info.tseq());
            }

            // new objects appear in to-space for generation 0
            for (Generation gi = g0; gi < gn; ++gi) {
                INFO(tostr(xtag("gi", gi)));

                if ((gi == 0) && (x1_v.size() > 0))
                    REQUIRE(gcos.to_space(gi)->allocated() > 0);
                else
                    REQUIRE(gcos.to_space(gi)->allocated() == 0);

                REQUIRE(gcos.from_space(gi)->allocated() == 0);
            }

            // swap_roles [but only for generation < g1, i.e. g0
            {
                gcos.swap_roles(g1);

                for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
                    const auto & x1 = x1_v.at(i);

                    REQUIRE(gcos.contains_allocated(Role::from_space(), x1.gco_.data()));
                    AllocInfo obj_info = gcos.alloc_info((std::byte *)x1.gco_.data());
                    REQUIRE(obj_info.size() >= x1.alloc_z_);

                    REQUIRE(obj_info.payload().first == (std::byte *)x1.gco_.data());
                    REQUIRE(obj_info.tseq() == x1.tseq_.seqno());

                    for (Generation gi = g0; gi < gn; ++gi) {
                        INFO(tostr(xtag("gi", gi)));

                        if (gi == 0)
                            REQUIRE(gcos.from_space(gi)->allocated() > 0);
                        else
                            REQUIRE(gcos.from_space(gi)->allocated() == 0);

                        REQUIRE(gcos.to_space(gi)->allocated() == 0);
                    }
                }
            }

            // try moving everything to to-space.
            // For this to week we must have registered the type,
            // so gc knows how to traverse it
            //
            for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
                const auto & x1 = x1_v.at(i);
                const auto & x2 = x2_v.at(i);

                INFO(tostr(xtag("i", i), xtag("n", n), xtag("x1.tseq_", x1.tseq_)));

                if (tc.do_type_registration_) {

                    /* Action of this loop iteration:
                     *
                     *          gcos                     arena2
                     *   +------------+-----------+    +--------+
                     *   |    from    |    to     |    |        |
                     *   |            |           |    |        |
                     *   |    +----+  |  +-----+  |    | +----+ |
                     *   |    | x1 |---->| x1p |  |    | | x2 | |
                     *   |    +----+  |  +-----+  |    | +----+ |
                     *   |            |           |    |        |
                     *   +------------+-----------+    +--------+
                     *
                     * Before:
                     *   x1, x2 have the same shape
                     * After
                     *   x1 forward to x1p
                     *   x1p and x2 have the same shape
                     */

                    // note: since members of x1_v[] can refer to each other,
                    // it's possible that x1.gco_ is already a forwarding pointer
                    // before we call deep_move_root().

                    AGCObject * x1p_iface = gcos.lookup_type(x1.tseq_);
                    REQUIRE(x1p_iface);

                    auto x1p_data = gcos.deep_move_root(mock_gc_visitor, x1.gco_, g1);
                    REQUIRE(x1p_data);

                    obj<AGCObject> x1p_gco(x1p_iface, x1p_data);

                    // obj has been replaced by forwarding pointer to obj2
                    {
                        REQUIRE(gcos.contains_allocated(Role::from_space(), x1.gco_.data()));
                        AllocInfo obj_info = gcos.alloc_info((std::byte *)x1.gco_.data());
                        REQUIRE(obj_info.size() >= x1.alloc_z_);

                        REQUIRE(obj_info.payload().first == (std::byte *)x1.gco_.data());
                        REQUIRE(obj_info.is_forwarding_tseq());
                    }

                    // obj1p same contents as original obj
                    {
                        REQUIRE(gcos.contains_allocated(Role::to_space(), x1p_gco.data()));
                        AllocInfo obj1p_info = gcos.alloc_info((std::byte *)x1p_gco.data());
                        REQUIRE(obj1p_info.size() >= x1.alloc_z_);

                        REQUIRE(obj1p_info.payload().first == (std::byte *)x1p_gco.data());
                        REQUIRE(obj1p_info.tseq() == x1.tseq_.seqno());
                    }

                    REQUIRE(x1p_gco.data() != nullptr);
                    REQUIRE(gcos.contains(Role::to_space(), x1p_gco.data()));
                    REQUIRE(gcos.contains_allocated(Role::to_space(), x1p_gco.data()));

                    // x1p_gco must look like x2.gco

                    REQUIRE(x1p_gco._typeseq() == x2.gco_._typeseq());

                    // written out polymorphic comparison
                    {
                        // match DBoolean..
                        bool match_attempted = false;
                        {
                            auto x1p_b = obj<AGCObject,DBoolean>::from(x1p_gco);
                            auto x2_b = obj<AGCObject,DBoolean>::from(x2.gco_);

                            if (x1p_b && x2_b) {
                                match_attempted = true;

                                REQUIRE(x1p_b->value() == x2_b->value());
                            }
                        }

                        // match DList..
                        {
                            auto x1p_b = obj<AGCObject,DList>::from(x1p_gco);
                            auto x2_b = obj<AGCObject,DList>::from(x2.gco_);

                            if (x1p_b && x2_b) {
                                match_attempted = true;

                                // TODO: we could figure out the index in {x1_v[], x2_v[]}
                                //       of x*_b {head, rest} respectively,
                                //       and verify they're consistent.

                                REQUIRE(x1p_b->head()._typeseq() == x2_b->head()._typeseq());
                                REQUIRE(x1p_b->size() == x2_b->size());

                                if (x1p_b->rest()) {
                                    REQUIRE(x2_b->rest());
                                } else {
                                    // unreachable, since using sentinel objectd for nil list
                                    REQUIRE(x2_b->rest() == nullptr);
                                }
                            }
                        }

                        REQUIRE(match_attempted);
                    }

                } else {
                    // can still try to move something.
                    // but will fail since type isn't registered

                    auto x1p_data = gcos.deep_move_root(mock_gc_visitor, x1.gco_, g1);

                    // control here under normal GC use
                    // would represent a configuration fail

                    REQUIRE(x1p_data == nullptr);
                }
            }

            // Things to test:
            // - deep_move_interior()   // used from MutationLogStore
            // - forward_inplace_aux()  // used from DX1Collector.visit_child
            // - cleanup_phase()        // used from DX1Collector._cleanup_phase

            // - report_object_types

        }
    }

} /*namespace ut*/

/* end GCObjectStore.test.cpp */
