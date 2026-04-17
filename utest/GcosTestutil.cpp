/** @file GcosTestutil.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GcosTestutil.hpp"
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/object2/ListOps.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/TypeRegistry.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <vector>
#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::ListOps;
    using xo::scm::DList;
    using xo::scm::DBoolean;
    using xo::mm::X1VerifyStats;
    using xo::mm::GCObjectStore;
    using xo::mm::AGCObject;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::AllocInfo;
    using xo::mm::Role;
    using xo::mm::Generation;
    using xo::mm::c_max_generation;
    using xo::mm::object_age;
    using xo::rng::xoshiro256ss;
    using xo::facet::TypeRegistry;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using xo::facet::impl_for;
    using xo::scope;
    using xo::xtag;

    /** Create two isomorphic object graphs.
     *  Each graph comprises a single DList cell
     *  that points to itself
     **/
    void
    GcosTestutil::selfcycle_object_graph(std::vector<Recd> * p_v1,
                                         GCObjectStore * p_gcos,
                                         std::vector<Recd> * p_v2,
                                         DArena * arena2)
    {
        auto alloc1 = obj<AAllocator,DArena>(p_gcos->new_space());
        auto alloc2 = obj<AAllocator,DArena>(arena2);

        auto t1 = DBoolean::box(alloc1, true);
        auto t2 = DBoolean::box(alloc2, true);

        auto l1 = ListOps::cons(alloc1, t1, ListOps::nil());
        auto l2 = ListOps::cons(alloc2, t2, ListOps::nil());

        // shortcut. Can get away with skipping mm_do_assign(),
        // because we know lhs of assignment is in the youngest generation

        l1->head_ = l1; // l1->assign_head(gc, l1); // need collector facet
        l2->head_ = l2; // l2->assign_head(gc, l2); // need collector facet

        p_v1->push_back(Recd(l1, sizeof(DList), typeseq::id<DList>()));
        p_v2->push_back(Recd(l2, sizeof(DList), typeseq::id<DList>()));
    }

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
    GcosTestutil::random_object_graph(uint32_t n_new_obj,
                                      uint32_t n_assign,
                                      xoshiro256ss * p_rgen,
                                      std::vector<Recd> * p_v,
                                      GCObjectStore * p_gcos,
                                      std::vector<Recd> * p_v2,
                                      DArena * p_arena2)
    {
        scope log(XO_DEBUG(true));

        if (n_new_obj == 0 && n_assign == 0)
            return;

        for (uint32_t i_obj = 0; i_obj < n_new_obj; ++i_obj) {
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
            uint32_t lhs_ix = (*p_rgen)() % p_v->size();

            assert(lhs_ix < p_v->size());

            // is it a list cell?
            auto xj1 = obj<AGCObject,DList>::from((*p_v)[lhs_ix].gco_);
            auto xj2 = obj<AGCObject,DList>::from((*p_v2)[lhs_ix].gco_);

            if (xj1) {
                assert(xj2);

                // flip a coin -- try modifying one of {car, cdr}
                uint32_t sample = (*p_rgen)() % 100;

                if (sample < 50) {
                    // modify head.  skip usual gc write-barrier stuff

                    uint32_t rhs_ix = (*p_rgen)() % p_v->size();

                    auto rhs1 = (*p_v)[rhs_ix].gco_;
                    auto rhs2 = (*p_v2)[rhs_ix].gco_;

                    if (log) {
                        log("replacing edge in random object graph");
                        log(xtag("n-obj", p_v->size()));
                        log(xtag("lhs-ix", lhs_ix));
                        log(xtag("rhs-ix", rhs_ix));
                        log(xtag("rhs.tname", TypeRegistry::id2name(rhs1._typeseq())));
                    }

                    // rhs1 could even be xj1 itself (in which case rhs2 is xj2)
                    xj1->head_ = rhs1;
                    xj2->head_ = rhs2;
                } else {
                    // don't modify DList.rest_, risks losing acyclic propertly.
                    // GCObjectStore handles this, but DList.size() assumes
                    // list is acyclic
                }
            }
        }
    } /*random_object_graph*/

    void
    GcosTestutil::gcos_install_test_types(bool do_type_registration,
                                          GCObjectStore * p_gcos)
    {
        // verify that GCOS recongnizes as registered,
        // the types we intend using for unit test

        if (do_type_registration) {
            {
                REQUIRE(p_gcos->install_type(impl_for<AGCObject,DBoolean>()));
                REQUIRE(p_gcos->is_type_installed(typeseq::id<DBoolean>()));
            }
            {
                REQUIRE(p_gcos->install_type(impl_for<AGCObject,DList>()));
                REQUIRE(p_gcos->is_type_installed(typeseq::id<DList>()));
            }
        }
    }

    void
    GcosTestutil::gcos_verify_arena_partitioning(uint32_t n_gen,
                                                 size_t gc_size,
                                                 const GCObjectStore & gcos)
    {
        Generation g0{0};
        Generation g1{1};
        Generation gn{n_gen};

        // verify basic arena partitioning + sizing

        REQUIRE(g0 != g1);
        REQUIRE(gcos.new_space());
        REQUIRE(gcos.new_space() == gcos.get_space(Role::to_space(), g0));
        REQUIRE(gcos.new_space()->reserved() >= gc_size);
        REQUIRE(gcos.from_space(g0));

        for (Generation gi = g1; gi < n_gen; ++gi) {
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

    void
    GcosTestutil::gcos_verify_vacant(uint32_t n_gen,
                                     size_t gc_size,
                                     const GCObjectStore & gcos)
    {
        Generation g0{0};
        Generation gn{n_gen};

        // verify we have non-zero space!
        {
            for (Generation gi = g0; gi < gn; ++gi) {
                INFO(tostr(xtag("gi", gi)));

                REQUIRE(gcos.to_space(gi)->allocated() == 0);
                REQUIRE(gcos.to_space(gi)->reserved() >= gc_size);

                REQUIRE(gcos.from_space(gi)->allocated() == 0);
                REQUIRE(gcos.from_space(gi)->reserved() >= gc_size);
            }
        }
    }

    /** Generate two copies of a random object graph for test case @p tc.
     *  Store first graph in @p *p_x1_v, allocating
     *  entirely from @p p_gcos new-space.
     *  Store second graph in @p *p_x2_v, allocating
     *  entirely from @p p_arena2.
     *  Use random number generator @p_rgen
     *
     *  @p loop_index counts iteration with one gc-like phase.
     **/
    void
    GcosTestutil::gcos_construct_ab_object_graphs(TestGraphType obj_graph_type,
                                                  uint32_t n_i0_test_obj,
                                                  uint32_t n_i0_test_assign,
                                                  uint32_t n_i1_test_obj,
                                                  uint32_t n_i1_test_assign,
                                                  GCObjectStore * p_gcos,
                                                  DArena * p_arena2,
                                                  uint32_t loop_index,
                                                  std::vector<Recd> * p_x1_v,
                                                  std::vector<Recd> * p_x2_v,
                                                  xoshiro256ss * p_rgen)
    {
        switch (obj_graph_type) {
        case TestGraphType::selfcycle:
            if (loop_index == 0) {
                GcosTestutil::selfcycle_object_graph(p_x1_v,
                                                     p_gcos,
                                                     p_x2_v,
                                                     p_arena2);
            }
            break;

        case TestGraphType::random:
            {
                uint32_t n_test_obj = ((loop_index == 0)
                                       ? n_i0_test_obj
                                       : n_i1_test_obj);
                uint32_t n_test_assign = ((loop_index == 0)
                                          ? n_i0_test_assign
                                          : n_i1_test_assign);

                GcosTestutil::random_object_graph(n_test_obj,
                                                  n_test_assign,
                                                  p_rgen,
                                                  p_x1_v,
                                                  p_gcos,
                                                  p_x2_v,
                                                  p_arena2);
            }
            break;
        }

        //x1_v.push_back(Recd(DBoolean::box(alloc, true),
        //                    sizeof(DBoolean),
        //                    typeseq::id<DBoolean>()));
    }

    void
    GcosTestutil::gcos_verify_consistency(GCObjectStore * p_gcos)
    {
        // traverses stored objects, updates counters
        // in verify_stats (= gco.p_verify_stats_, via ctor)
        //
        p_gcos->verify_ok();

        X1VerifyStats * verify_stats = p_gcos->verify_stats();

        INFO(tostr(xtag("n_gc_root", verify_stats->n_gc_root_),
                   xtag("n_ext", verify_stats->n_ext_),
                   xtag("n_from", verify_stats->n_from_),
                   xtag("n_to", verify_stats->n_to_),
                   xtag("n_fwd", verify_stats->n_fwd_),
                   xtag("n_age_ok", verify_stats->n_age_ok_),
                   xtag("n_age_bad", verify_stats->n_age_bad_),
                   xtag("n_no_iface", verify_stats->n_no_iface_)));

        REQUIRE(verify_stats->is_ok());
    }

    void
    GcosTestutil::gcos_verify_ab_equivalence(const std::vector<Recd> & x1_v,
                                             const std::vector<Recd> & x2_v)
    {
        REQUIRE(x1_v.size() == x2_v.size());

        for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
            REQUIRE(x1_v[i].alloc_z_ == x2_v[i].alloc_z_);
            REQUIRE(x1_v[i].tseq_ == x2_v[i].tseq_);

            REQUIRE(x1_v[i].gco_._typeseq() == x1_v[i].tseq_);
            REQUIRE(x2_v[i].gco_._typeseq() == x2_v[i].tseq_);
        }
    }

    void
    GcosTestutil::gcos_verify_allocinfo(const GCObjectStore & gcos,
                                        uint32_t loop_index,
                                        const std::vector<Recd> & x1_v)
    {
        // gcos can reveal info about allocs
        for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
            const auto & x1 = x1_v.at(i);

            REQUIRE(gcos.contains_allocated(Role::to_space(), x1.gco_.data()));
            AllocInfo obj_info = gcos.alloc_info((std::byte *)x1.gco_.data());
            REQUIRE(obj_info.size() >= x1.alloc_z_);

            REQUIRE(obj_info.payload().first == (std::byte *)x1.gco_.data());
            REQUIRE(obj_info.tseq() == x1.tseq_.seqno());

            // also can use header2size / header2tseq convenience functions
            REQUIRE(gcos.header2size(obj_info.header()) == obj_info.size());
            REQUIRE(gcos.header2age(obj_info.header()) <= object_age{loop_index});
            REQUIRE(gcos.header2tseq(obj_info.header()) == obj_info.tseq());
            REQUIRE(gcos.is_forwarding_header(obj_info.header()) == false);
        }
    }

    void
    GcosTestutil::gcos_verify_gen0_only_allocated(uint32_t n_gen,
                                                  const GCObjectStore & gcos,
                                                  uint32_t loop_index,
                                                  const std::vector<Recd> & x1_v)
    {
        Generation g0{0};
        Generation gn{n_gen};

        // new objects appear in to-space for generation 0.
        for (Generation gi = g0; gi < gn; ++gi) {
            INFO(tostr(xtag("gi", gi)));

            if (loop_index == 0) {
                if ((gi == 0) && (x1_v.size() > 0))
                    REQUIRE(gcos.to_space(gi)->allocated() > 0);
                else
                    REQUIRE(gcos.to_space(gi)->allocated() == 0);
            }

            REQUIRE(gcos.from_space(gi)->allocated() == 0);
        }
    }

} /*namespace ut*/

/* end GcosTestutil.cpp */
