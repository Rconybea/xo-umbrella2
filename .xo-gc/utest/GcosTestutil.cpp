/** @file GcosTestutil.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GcosTestutil.hpp"
#include "MockCollector.hpp"
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/object2/ListOps.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/alloc2/Collector.hpp>
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
    using xo::scm::DInteger;
    using xo::mm::ACollector;
    using xo::mm::DMockCollector;
    using xo::mm::X1VerifyStats;
    using xo::mm::MutationLog;
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
                                      bool debug_flag,
                                      xoshiro256ss * p_rgen,
                                      std::vector<Recd> * p_v,
                                      GCObjectStore * p_gcos,
                                      std::vector<Recd> * p_v2,
                                      DArena * p_arena2)
    {
        scope log(XO_DEBUG(debug_flag));

        if (n_new_obj == 0 && n_assign == 0)
            return;

        // TODO: combine // alloc setup w/ gco_construct_ab_object_graphs() bolierplate

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
                REQUIRE(p_gcos->install_type(impl_for<AGCObject,DInteger>()));
                REQUIRE(p_gcos->is_type_installed(typeseq::id<DInteger>()));
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
    GcosTestutil::gcos_construct_ab_object_graphs(TestSequence test_seq,
                                                  TestGraphType obj_graph_type,
                                                  uint32_t n_i0_test_obj,
                                                  uint32_t n_i0_test_assign,
                                                  uint32_t n_i1_test_obj,
                                                  uint32_t n_i1_test_assign,
                                                  bool debug_flag,
                                                  MutationLogStore * p_mls,
                                                  GCObjectStore * p_gcos,
                                                  DArena * p_arena2,
                                                  uint32_t loop_index,
                                                  std::vector<Recd> * p_x1_v,
                                                  std::vector<Recd> * p_x2_v,
                                                  xoshiro256ss * p_rgen)
    {
        /**  TestSequence memory layout
         *
         *  test_seq
         *    |
         *    v
         *    TestSequence
         *    +-----------+     cmd_seq_[] (shared, sentinel-terminated)
         *    | cmd_seq_  |---> +-------+-------+-------+-------+-------+-------+
         *    +-----------+     | step0 | step1 | step2 | step3 | step4 | SENTL |
         *    | phases_   |-\   +-------+-------+-------+-------+-------+-------+
         *    +-----------+ |   ix: 0       1       2       3       4       5
         *                  |
         *                  |   phases_[] (sentinel-terminated)
         *                  \-> +-----------+-----------+
         *                      |  Phase 0  |  SENTINEL |
         *                      | lo_ix_=0  | lo_ix_=-1 |
         *                      | hi_ix_=5  | hi_ix_=-1 |
         *                      | mlog_new_ | mlog_new_ |
         *                      +-----------+-----------+
         *
         *  Phase.lo_ix_ / hi_ix_ index into test_seq->cmd_seq_[].
         *  Phase 0 executes cmd_seq_[lo_ix_ .. hi_ix_), i.e. steps 0..4.
         *  Sentinel phase has lo_ix_ == -1.
         *
         *  Each Step has {cmd_, arg0_ix_, arg1_ix_}.
         *  arg0_ix_ and arg1_ix_ index into x1_v[] / x2_v[],
         *  referring to objects created by earlier steps.
         *
         *  Example (seq1):
         *    step0: {make_bool,   0, 0}  -> x1_v[0] = #f
         *    step1: {make_bool,   1, 0}  -> x1_v[1] = #t
         *    step2: {make_nil,    0, 0}  -> x1_v[2] = ()
         *    step3: {make_cons,   0, 2}  -> x1_v[3] = cons(x1_v[0], x1_v[2]) = (#f)
         *    step4: {assign_head, 3, 1}  -> set-car!(x1_v[3], x1_v[1])  => (#t)
         **/

        if (!test_seq.is_sentinel()) {
            // Explicit command sequence.
            // Each command creates a new node or modifies an existing one

            // 1. Sequence of commands for this call.
            //    Will be phases[loop_index] if well-defined.
            // 2. Expected effect on mutation log
            //
            Phase * phase_expect = nullptr;
            {
                Phase * p_phase = test_seq.phases_;

                if (test_seq.phases_) {
                    for (uint32_t i = 0; i < loop_index; ++i) {
                        if (!p_phase->is_sentinel())
                            ++p_phase;
                        else
                            p_phase = nullptr;
                    }
                }

                phase_expect = p_phase;
            }

            Step * cmd_seq = test_seq.cmd_seq_;

            if (phase_expect && cmd_seq) {
                // Do scripted sequence only.
                // For this phases that is
                //   cmd_seq[ix]
                // for
                //   phase_expect->lo_ix_ <= ix < phase_expect->hi_ix_

                auto alloc = obj<AAllocator,DArena>(p_gcos->new_space());
                auto alloc2 = obj<AAllocator,DArena>(p_arena2);
                DMockCollector mock(p_mls, p_gcos);
                auto mockgc = obj<ACollector,DMockCollector>(&mock);

                for (int32_t ix = phase_expect->lo_ix_, hi = phase_expect->hi_ix_; ix < hi; ++ix) {
                    const Step & cmd = cmd_seq[ix];

                    bool is_alloc = false;
                    obj<AGCObject> xi;
                    obj<AGCObject> xi2;
                    uint64_t alloc_z = 0;
                    typeseq tseq;

                    switch (cmd.cmd_) {
                    case Step::Cmd::sentinel:
                        assert(false); // unreachable
                        break;
                    case Step::Cmd::make_nil:
                        // TODO combine with code in random_object_graph()
                        {
                            is_alloc = true;

                            xi = ListOps::nil();
                            alloc_z = 0;  // not in gcos space
                            tseq = typeseq::id<DList>();

                            xi2 = ListOps::nil();

                            REQUIRE(xi._typeseq() == tseq);
                            REQUIRE(xi2._typeseq() == tseq);
                        }
                        break;
                    case Step::Cmd::make_cons:
                        // TODO combine with code in random_object_graph()
                        {
                            auto h1 = p_x1_v->at(cmd.arg0_ix_).gco_;
                            auto r1 = obj<AGCObject,DList>::from(p_x1_v->at(cmd.arg1_ix_).gco_);
                            auto h2 = p_x2_v->at(cmd.arg0_ix_).gco_;
                            auto r2 = obj<AGCObject,DList>::from(p_x2_v->at(cmd.arg1_ix_).gco_);

                            is_alloc = true;

                            xi = ListOps::cons(alloc, h1, r1);
                            alloc_z = sizeof(DList);
                            tseq = typeseq::id<DList>();

                            xi2 = ListOps::cons(alloc2, h2, r2);
                        }
                        break;
                    case Step::Cmd::make_bool:
                        // TODO combine with code in random_object_graph()
                        {
                            bool value = (cmd.arg0_ix_ > 0);

                            is_alloc = true;

                            xi = DBoolean::box(alloc, value);
                            alloc_z = sizeof(DBoolean);
                            tseq = typeseq::id<DBoolean>();

                            xi2 = DBoolean::box(alloc2, value);
                        }
                        break;
                    case Step::Cmd::make_int:
                        {
                            int value = cmd.arg0_ix_;

                            is_alloc = true;

                            xi = DInteger::box(alloc, value);
                            alloc_z = sizeof(DInteger);
                            tseq = typeseq::id<DInteger>();

                            xi2 = DInteger::box(alloc2, value);
                        }
                        break;
                    case Step::Cmd::assign_root:
                        {
                            is_alloc = false;

                            auto z1 = p_x1_v->at(cmd.arg1_ix_).alloc_z_;
                            auto tseq1 = p_x1_v->at(cmd.arg1_ix_).tseq_;
                            auto rhs1 = p_x1_v->at(cmd.arg1_ix_).gco_;

                            auto z2 = p_x2_v->at(cmd.arg1_ix_).alloc_z_;
                            auto tseq2 = p_x2_v->at(cmd.arg1_ix_).tseq_;
                            auto rhs2 = p_x2_v->at(cmd.arg1_ix_).gco_;

                            p_x1_v->at(cmd.arg0_ix_).alloc_z_ = z1;
                            p_x1_v->at(cmd.arg0_ix_).tseq_ = tseq1;
                            p_x1_v->at(cmd.arg0_ix_).gco_ = rhs1;

                            p_x2_v->at(cmd.arg0_ix_).alloc_z_ = z2;
                            p_x2_v->at(cmd.arg0_ix_).tseq_ = tseq2;
                            p_x2_v->at(cmd.arg0_ix_).gco_ = rhs2;
                        }
                        break;
                    case Step::Cmd::assign_head:
                        {
                            is_alloc = false;

                            auto lhs1 = obj<AGCObject,DList>::from(p_x1_v->at(cmd.arg0_ix_).gco_);
                            auto rhs1 = p_x1_v->at(cmd.arg1_ix_).gco_;

                            auto lhs2 = obj<AGCObject,DList>::from(p_x2_v->at(cmd.arg0_ix_).gco_);
                            auto rhs2 = p_x2_v->at(cmd.arg1_ix_).gco_;

                            assert(lhs1);
                            assert(!lhs1->is_empty());

                            assert(lhs2);
                            assert(!lhs2->is_empty());

                            assert(p_mls);
                            assert(mockgc);

                            lhs1->assign_head_gc(mockgc, rhs1);
                            // alloc2 is ord arena -> no mlog
                        }
                        break;
                    }

                    if (is_alloc) {
                        p_x1_v->push_back(Recd(xi,  alloc_z, tseq));
                        p_x2_v->push_back(Recd(xi2, alloc_z, tseq));
                    }
                }

                // check expected results

                for (Generation gi{0}; gi + 1 < Generation(p_mls->config().n_generation_); ++gi) {
                    MutationLog * mlog = p_mls->get_mlog(Role::to_space(), gi);

                    REQUIRE(mlog);
                    REQUIRE(mlog->size() == phase_expect->mlog_new_z_[gi]);
                }
            }
        } else {
            switch (obj_graph_type) {
            case TestGraphType::fixed:
                assert(false); // unreachable
                break;

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
                                                      debug_flag,
                                                      p_rgen,
                                                      p_x1_v,
                                                      p_gcos,
                                                      p_x2_v,
                                                      p_arena2);
                }
                break;
            }
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

            // x1 could be a global, such as ListOps::nil()
            if (x1.alloc_z_ > 0) {
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
            } else {
                REQUIRE(!gcos.contains(Role::to_space(), x1.gco_.data()));
                REQUIRE(!gcos.contains(Role::from_space(), x1.gco_.data()));
            }
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
                if ((gi == 0) && (x1_v.size() > 0)) {
                    // conceivable that x1_v[] only contains non-gco objects
                    //REQUIRE(gcos.to_space(gi)->allocated() > 0);
                } else {
                    REQUIRE(gcos.to_space(gi)->allocated() == 0);
                }
            }

            REQUIRE(gcos.from_space(gi)->allocated() == 0);
        }
    }

    void
    GcosTestutil::gcos_verify_gen0_fromspace_only_allocated(uint32_t n_gen,
                                                            const GCObjectStore & gcos,
                                                            uint32_t loop_index,
                                                            Generation upto,
                                                            const std::vector<Recd> & x1_v)
    {
        Generation g0{0};
        Generation gn{n_gen};

        for (Generation gi = g0; gi < gn; ++gi) {
            if (gi < upto) {
                // we're collecting generation gi.
                // Before we begin, to-space had better be empty
                // (everthing in gi is in from-space)

                REQUIRE(gcos.to_space(gi)->allocated() == 0);
            } else {
                // we're not collecting generation gi.
                // from-space must be empty.
                // May have content in to-space

                REQUIRE(gcos.from_space(gi)->allocated() == 0);
            }
        }

        for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
            const auto & x1 = x1_v.at(i);

            // x1 should be in gen g from-space (with g < upto)
            // or in gen g to-space (with g >= upto)

            if (x1.alloc_z_ > 0) {
                Generation g_from = gcos.generation_of(Role::from_space(), x1.gco_.data());
                Generation g_to = gcos.generation_of(Role::to_space(), x1.gco_.data());

                if (g_to.is_sentinel()) {
                    // if not in to-space, must be in from-space
                    REQUIRE(!g_from.is_sentinel());

                    // + for some gen we're collecting
                    REQUIRE(g_from < upto);

                    REQUIRE(gcos.contains(Role::from_space(), x1.gco_.data()));
                    REQUIRE(gcos.contains_allocated(Role::from_space(), x1.gco_.data()));
                } else {
                    // if in to-space, must not be in from-space
                    REQUIRE(g_from.is_sentinel());

                    // + for some gen we're not collecting
                    REQUIRE(g_to >= upto);

                    REQUIRE(gcos.contains(Role::to_space(), x1.gco_.data()));
                    REQUIRE(gcos.contains_allocated(Role::to_space(), x1.gco_.data()));
                }

                AllocInfo obj_info = gcos.alloc_info((std::byte *)x1.gco_.data());
                REQUIRE(obj_info.size() >= x1.alloc_z_);

                REQUIRE(obj_info.payload().first == (std::byte *)x1.gco_.data());
                REQUIRE(obj_info.tseq() == x1.tseq_.seqno());
            } else {
                REQUIRE(!gcos.contains(Role::to_space(), x1.gco_.data()));
                REQUIRE(!gcos.contains(Role::from_space(), x1.gco_.data()));
            }
        }
    }

    void
    GcosTestutil::gcos_move_roots_and_verify(bool do_type_registration,
                                             GCObjectStore * p_gcos,
                                             Generation upto,
                                             const std::vector<Recd> & x1_v,
                                             const std::vector<Recd> & x2_v,
                                             bool debug_flag)
    {
        scope log(XO_DEBUG(debug_flag));

        Generation g1{1};

        // try moving everything to to-space.
        // For this to week we must have registered the type,
        // so gc knows how to traverse it
        //
        for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
            const auto & x1 = x1_v.at(i);
            const auto & x2 = x2_v.at(i);

            log && log("moving roots");
            log && log(xtag("i", i),
                       xtag("n", n),
                       xtag("x1.tseq_", x1.tseq_),
                       xtag("x1.tname", TypeRegistry::id2name(x1.tseq_)));

            if (do_type_registration) {

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

                AGCObject * x1p_iface = p_gcos->lookup_type(x1.tseq_);
                REQUIRE(x1p_iface);

                // snapshot root before moving
                obj<AGCObject> x1_gco = x1.gco_;

                // modifies x1.gco_ in place
                auto x1p_data
                    = p_gcos->deep_move_root(x1p_iface, (void **)&(x1.gco_.data_), upto);

                REQUIRE(x1p_data);
                REQUIRE(x1p_data == x1.gco_.data_);

                obj<AGCObject> x1p_gco(x1p_iface, x1p_data);

                // obj (x1_gco) now forwarding pointer (to x1p_gco = x1.gco_)
                GcosTestutil::gcos_verify_forwarding(*p_gcos, upto, x1, x1_gco);

                // obj1p in to-space, same contents as original obj
                gcos_verify_forwarding_destination(*p_gcos, x1, x1p_gco);

                // x1p_gco must look like x2.gco
                REQUIRE(x1p_gco._typeseq() == x2.gco_._typeseq());

                gcos_verify_forwarded_ab_equivalence(x1p_gco, x2.gco_);
            } else {
                // can still try to move something.
                // but will fail since type isn't registered

                auto x1p_data
                    = p_gcos->deep_move_root(x1.gco_.iface(),
                                             (void **)&(x1.gco_.data_),
                                             g1);

                // control here under normal GC use
                // would represent a configuration fail

                REQUIRE(x1p_data == nullptr);
            }
        }
    }

    void
    GcosTestutil::gcos_verify_forwarding(const GCObjectStore & gcos,
                                         Generation upto,
                                         const Recd & x1,
                                         obj<AGCObject> x1_gco)
    {
        if (x1.alloc_z_ > 0) {
            REQUIRE((gcos.contains_allocated(Role::from_space(), x1_gco.data())
                     || gcos.contains_allocated(Role::to_space(), x1_gco.data())));
            AllocInfo obj_info = gcos.alloc_info((std::byte *)x1_gco.data());

            INFO(tostr(xtag("obj_info.tseq", obj_info.tseq()),
                       xtag("obj_info.tname", TypeRegistry::id2name(typeseq(obj_info.tseq())))));

            REQUIRE(obj_info.size() >= x1.alloc_z_);
            REQUIRE(obj_info.payload().first == (std::byte *)x1_gco.data());

            if (obj_info.is_forwarding_tseq()) {
                /* object was forwarded, so got collected */
                REQUIRE(obj_info.is_forwarding_tseq());
            } else {
                /* not forwarded is ok iff in generation g >= upto */

                Generation g = gcos.generation_of(Role::to_space(), x1_gco.data());

                REQUIRE(g >= upto);
            }

            //            if (!obj_info.is_forwarding_tseq())
            //                print_backtrace_dwarf(true /*demangle*/);
            //            REQUIRE(obj_info.is_forwarding_tseq());
        }
    }

    void
    GcosTestutil::gcos_verify_forwarding_destination(const GCObjectStore & gcos,
                                                     const Recd & x1,
                                                     obj<AGCObject> x1p_gco)
    {
        if (x1.alloc_z_ > 0) {
            REQUIRE(gcos.contains_allocated(Role::to_space(), x1p_gco.data()));
            AllocInfo obj1p_info = gcos.alloc_info((std::byte *)x1p_gco.data());
            REQUIRE(obj1p_info.size() >= x1.alloc_z_);

            REQUIRE(obj1p_info.payload().first == (std::byte *)x1p_gco.data());
            REQUIRE(obj1p_info.tseq() == x1.tseq_.seqno());

            REQUIRE(x1p_gco.data() != nullptr);
            REQUIRE(gcos.contains(Role::to_space(), x1p_gco.data()));
            REQUIRE(gcos.contains_allocated(Role::to_space(), x1p_gco.data()));
        }
    }

    void
    GcosTestutil::gcos_verify_forwarded_ab_equivalence(obj<AGCObject> x1p_gco,
                                                       obj<AGCObject> x2_gco)
    {
        // written out polymorphic comparison

        bool match_attempted = false;

        // match DBoolean..
        {
            auto x1p_b = obj<AGCObject,DBoolean>::from(x1p_gco);
            auto x2_b = obj<AGCObject,DBoolean>::from(x2_gco);

            if (x1p_b && x2_b) {
                match_attempted = true;

                REQUIRE(x1p_b->value() == x2_b->value());
            }
        }

        // match DInteger..
        {
            auto x1p_b = obj<AGCObject,DInteger>::from(x1p_gco);
            auto x2_b = obj<AGCObject,DInteger>::from(x2_gco);

            if (x1p_b && x2_b) {
                match_attempted = true;

                REQUIRE(x1p_b->value() == x2_b->value());
            }
        }

        // match DList..
        {
            auto x1p_b = obj<AGCObject,DList>::from(x1p_gco);
            auto x2_b = obj<AGCObject,DList>::from(x2_gco);

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

} /*namespace ut*/

/* end GcosTestutil.cpp */
