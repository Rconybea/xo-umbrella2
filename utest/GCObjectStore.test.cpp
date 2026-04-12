/** @file GCObjectStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include <xo/gc/GCObjectStore.hpp>
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/object2/ListOps.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/TypeRegistry.hpp>
#include <xo/arena/print.hpp>
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
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::GCObjectStore;
    using xo::mm::X1VerifyStats;
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
    using xo::facet::TypeRegistry;
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
        enum class TestGraphType {
            /* list cell pointing to itself */
            selfcycle,
            /* random object graph */
            random,
        };

        struct Testcase {
            explicit Testcase(uint32_t n_gen, uint32_t n_survive,
                              size_t gc_z, uint32_t type_z,
                              bool do_type_registration,
                              size_t report_z,
                              size_t error_z,
                              TestGraphType obj_graph_type,
                              uint32_t n_test_obj,
                              uint32_t n_test_assign,
                              bool debug_flag)
                : n_gen_{n_gen},
                  n_survive_{n_survive},
                  gc_size_{gc_z},
                  object_type_z_{type_z},
                  do_type_registration_{do_type_registration},
                  report_size_{report_z},
                  error_size_{error_z},
                  obj_graph_type_{obj_graph_type},
                  n_test_obj_{n_test_obj},
                  n_test_assign_{n_test_assign},
                  debug_flag_{debug_flag}
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
            /** size for report-output arena **/
            size_t report_size_ = 0;
            /** size for error-output arena **/
            size_t error_size_ = 0;
            /** object graph type **/
            TestGraphType obj_graph_type_ = TestGraphType::random;
            /** #of cells in random object graph **/
            uint32_t n_test_obj_ = 0;
            /** #of random assignments to attempt (these may create cycles, for example) **/
            uint32_t n_test_assign_ = 0;

            /** true to enable debug when attempting this test case **/
            bool debug_flag_ = false;
        };

        constexpr TestGraphType c_selfcycle = TestGraphType::selfcycle;
        constexpr TestGraphType c_random = TestGraphType::random;
        constexpr uint32_t c_report_z1 = 64 * 1024;
        constexpr uint32_t c_error_z1 = 16 * 1024;

        static std::vector<Testcase> s_testcase_v = {
            // note: report_z: 64k not sufficient for report_object_ages()

            /** n_gen, n_survive, gc_size, object_type_z, do_type_registration, report_z, error_z, n_obj, n_test_assign **/
            Testcase(2, 4, 16 * 1024, 8 * 128, false, c_report_z1, c_error_z1, c_random,    0, 0, false), 
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_selfcycle, 1, 0, false), 
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_random,    1, 0, false), 
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_random,    2, 13, false),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_random,    2, 25, false),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_random,    5,  0, false),
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_random,    4,  2, false), 
            Testcase(2, 4, 16 * 1024, 8 * 128, true,  c_report_z1, c_error_z1, c_random,   50, 25, false), 
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

        /** Create two isomorphic object graphs.
         *  Each graph comprises a single DList cell
         *  that points to itself
         **/
        void
        selfcycle_object_graph(std::vector<Recd> * p_v1,
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
        random_object_graph(uint32_t n_obj,
                            uint32_t n_assign,
                            xoshiro256ss * p_rgen,
                            std::vector<Recd> * p_v,
                            GCObjectStore * p_gcos,
                            std::vector<Recd> * p_v2,
                            DArena * p_arena2)
        {
            scope log(XO_DEBUG(true));

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
                uint32_t lhs_ix = (*p_rgen)() % n_obj;

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

                        uint32_t rhs_ix = (*p_rgen)() % n_obj;

                        auto rhs1 = (*p_v)[rhs_ix].gco_;
                        auto rhs2 = (*p_v2)[rhs_ix].gco_;

                        if (log) {
                            log("replacing edge in random object graph");
                            log(xtag("n-obj", n_obj));
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
    } /*namespace*/

    namespace {
        // aux functions specific to GCObjectStore-1 unit test below

        void
        gcos_install_test_types(const Testcase & tc,
                                GCObjectStore * p_gcos)
        {
            // verify that GCOS recongnizes as registered,
            // the types we intend using for unit test
            
            if (tc.do_type_registration_) {
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
        gcos_verify_arena_partitioning(const Testcase & tc,
                                       const GCObjectStore & gcos)
        {
            Generation g0{0};
            Generation g1{1};
            Generation gn{tc.n_gen_};

            // verify basic arena partitioning + sizing

            REQUIRE(g0 != g1);
            REQUIRE(gcos.new_space());
            REQUIRE(gcos.new_space() == gcos.get_space(Role::to_space(), g0));
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

        void
        gcos_verify_vacant(const Testcase & tc,
                           const GCObjectStore & gcos)
        {
            Generation g0{0};
            Generation gn{tc.n_gen_};


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
        }

        /** Generate two copies of a random object graph for test case @p tc.
         *  Store first graph in @p *p_x1_v, allocating
         *  entirely from @p p_gcos new-space.
         *  Store second graph in @p *p_x2_v, allocating
         *  entirely from @p p_arena2.
         *  Use random number generator @p_rgen
         **/
        void
        gcos_construct_ab_object_graphs(const Testcase & tc,
                                        GCObjectStore * p_gcos,
                                        DArena * p_arena2,
                                        std::vector<Recd> * p_x1_v,
                                        std::vector<Recd> * p_x2_v,
                                        xoshiro256ss * p_rgen)
        {
            switch (tc.obj_graph_type_) {
            case TestGraphType::selfcycle:
                selfcycle_object_graph(p_x1_v,
                                       p_gcos,
                                       p_x2_v,
                                       p_arena2);
                break;
            case TestGraphType::random:
                random_object_graph(tc.n_test_obj_,
                                    tc.n_test_assign_,
                                    p_rgen,
                                    p_x1_v,
                                    p_gcos,
                                    p_x2_v,
                                    p_arena2);
                break;
            }

            //x1_v.push_back(Recd(DBoolean::box(alloc, true),
            //                    sizeof(DBoolean),
            //                    typeseq::id<DBoolean>()));
        }

        /** Invoke built-in consistency verification for @p *p_gcos.
         **/
        void
        gcos_verify_consistency(GCObjectStore * p_gcos)
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
                       xtag("n_no_iface", verify_stats->n_no_iface_)));

            REQUIRE(verify_stats->is_ok());
        }

        void
        gcos_verify_ab_equivalence(const std::vector<Recd> & x1_v,
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
        gcos_verify_allocinfo(const GCObjectStore & gcos,
                              const std::vector<Recd> & x1_v)
        {
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
                REQUIRE(gcos.is_forwarding_header(obj_info.header()) == false);
            }
        }

        void
        gcos_verify_gen0_only_allocated(const Testcase & tc,
                                        const GCObjectStore & gcos,
                                        const std::vector<Recd> & x1_v)
        {
            Generation g0{0};
            Generation gn{tc.n_gen_};

            // new objects appear in to-space for generation 0
            for (Generation gi = g0; gi < gn; ++gi) {
                INFO(tostr(xtag("gi", gi)));

                if ((gi == 0) && (x1_v.size() > 0))
                    REQUIRE(gcos.to_space(gi)->allocated() > 0);
                else
                    REQUIRE(gcos.to_space(gi)->allocated() == 0);

                REQUIRE(gcos.from_space(gi)->allocated() == 0);
            }
        }

        void
        gcos_verify_gen0_fromspace_only_allocated(const Testcase & tc,
                                                  const GCObjectStore & gcos,
                                                  const std::vector<Recd> & x1_v)
        {
            for (size_t i = 0, n = x1_v.size(); i < n; ++i) {
                const auto & x1 = x1_v.at(i);

                REQUIRE(gcos.contains(Role::from_space(), x1.gco_.data()));
                REQUIRE(gcos.contains_allocated(Role::from_space(), x1.gco_.data()));
                AllocInfo obj_info = gcos.alloc_info((std::byte *)x1.gco_.data());
                REQUIRE(obj_info.size() >= x1.alloc_z_);

                REQUIRE(obj_info.payload().first == (std::byte *)x1.gco_.data());
                REQUIRE(obj_info.tseq() == x1.tseq_.seqno());

                Generation g0{0};
                Generation gn{tc.n_gen_};

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

        void
        gcos_verify_forwarding(const GCObjectStore & gcos,
                               const Recd & x1,
                               obj<AGCObject> x1_gco)
        {
            REQUIRE(gcos.contains_allocated(Role::from_space(), x1_gco.data()));
            AllocInfo obj_info = gcos.alloc_info((std::byte *)x1_gco.data());
            REQUIRE(obj_info.size() >= x1.alloc_z_);

            REQUIRE(obj_info.payload().first == (std::byte *)x1_gco.data());
            REQUIRE(obj_info.is_forwarding_tseq());
        }

        void
        gcos_verify_forwarding_destination(const GCObjectStore & gcos,
                                           const Recd & x1,
                                           obj<AGCObject> x1p_gco)
        {
            REQUIRE(gcos.contains_allocated(Role::to_space(), x1p_gco.data()));
            AllocInfo obj1p_info = gcos.alloc_info((std::byte *)x1p_gco.data());
            REQUIRE(obj1p_info.size() >= x1.alloc_z_);

            REQUIRE(obj1p_info.payload().first == (std::byte *)x1p_gco.data());
            REQUIRE(obj1p_info.tseq() == x1.tseq_.seqno());

            REQUIRE(x1p_gco.data() != nullptr);
            REQUIRE(gcos.contains(Role::to_space(), x1p_gco.data()));
            REQUIRE(gcos.contains_allocated(Role::to_space(), x1p_gco.data()));
        }

        void
        gcos_verify_forwarded_ab_equivalence(obj<AGCObject> x1p_gco,
                                             obj<AGCObject> x2_gco)
        {
            // written out polymorphic comparison

            // match DBoolean..
            bool match_attempted = false;
            {
                auto x1p_b = obj<AGCObject,DBoolean>::from(x1p_gco);
                auto x2_b = obj<AGCObject,DBoolean>::from(x2_gco);

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

        void
        gcos_move_roots_and_verify(const Testcase & tc,
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
                    gcos_verify_forwarding(*p_gcos, x1, x1_gco);

                    // obj1p same contents as original obj
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

        // fixture for GCObjectStore-1 test
        class GcosFixture {
        public:
            explicit GcosFixture(const Testcase & tc);

            GCObjectStoreConfig gcos_config_;

            /** Parallel arena for reference
             *
             *  We will allocate parallel object model in this arena
             *  for reference; then compare with GCObjectStore behavior.
             *
             *  1. arena2 doesn't have any generation layer cake stuff
             *  2. arena2 doesn't have concept of installed types.
             *     It doesn't have or require any builtin ability to traverse an object model
             **/
            DArena arena2_;
            /** Arena for holding report output:
             *  See GCObjectStore methods .report_object_types(), .report_object_ages()
             **/
            DArena report_arena_;
            /** Arena for holding error messages **/
            DArena error_arena_;

            /** statistics collected by GCObjectStore.verify_ok() **/
            X1VerifyStats verify_stats_;
        };

        GcosFixture::GcosFixture(const Testcase & tc)
        : gcos_config_{ArenaConfig()
                       .with_name("gcos-fixture-arena-name-notused")
                       .with_size(tc.gc_size_)
                       .with_store_header_flag(true),
                       tc.n_gen_,
                       tc.n_survive_,
                       tc.object_type_z_,
                       tc.debug_flag_},
          arena2_{DArena::map(ArenaConfig().with_name("arena2-ref")
                              .with_size(tc.gc_size_ * tc.n_gen_)
                              .with_store_header_flag(true))},
          report_arena_{DArena::map(ArenaConfig().with_name("report-arena")
                                    .with_size(tc.report_size_)
                                    .with_store_header_flag(true))},
          error_arena_{DArena::map(ArenaConfig().with_name("error-arena")
                                   .with_size(tc.error_size_)
                                   .with_store_header_flag(true))}
        {}
        
    }

    TEST_CASE("GCObjectStore-1", "[GCObjectStore]")
    {
        constexpr bool c_debug_flag = true;
        scope log0(XO_DEBUG(c_debug_flag), "GCObjectStore test");

        std::uint64_t seed = 12168164826603821466ul;
        //random_seed(&seed);
        log0 && log0(xtag("seed", seed));

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            // Loop iterations here are independent.
            // Could execute test cases in any order

            // deterministic seed choice for each testcase
            // -> individual cases preserve rng behavior
            // regardless of testcase order and/or subsetting
            
            auto rgen = xoshiro256ss(seed + i_tc);

            const Testcase & tc = s_testcase_v[i_tc];

            scope log1(XO_DEBUG(tc.debug_flag_), "testcase loop", xtag("i_tc", i_tc));

            INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

            GcosFixture fixture(tc);

            obj<AAllocator,DArena> report_mm(&fixture.report_arena_);
            obj<AAllocator,DArena> error_mm(&fixture.error_arena_);

            // object type storage will be empty unless we install a type.
            GCObjectStore gcos(fixture.gcos_config_, &fixture.verify_stats_);

            REQUIRE(gcos.is_type_installed(typeseq::id<DList>()) == false);
            REQUIRE(gcos.is_type_installed(typeseq::id<DBoolean>()) == false);

            gcos_install_test_types(tc, &gcos);
            gcos_verify_arena_partitioning(tc, gcos);
            gcos_verify_vacant(tc, gcos);

            // create object(s).
            // details depend on test case

            std::vector<Recd> x1_v;
            std::vector<Recd> x2_v;

            gcos_construct_ab_object_graphs(tc, &gcos, &fixture.arena2_, &x1_v, &x2_v, &rgen);

            log1 && log1("verify before any gcos side effects");

            gcos_verify_consistency(&gcos);

            // someday: print the graph. Need a cycle-detecting printer

            gcos_verify_ab_equivalence(x1_v, x2_v);
            gcos_verify_allocinfo(gcos, x1_v);
            gcos_verify_gen0_only_allocated(tc, gcos, x1_v);

            // swap_roles [but only for generation < g1, i.e. g0
            gcos.swap_roles(Generation::g1());

            gcos_verify_gen0_fromspace_only_allocated(tc, gcos, x1_v);

            gcos_move_roots_and_verify(tc, &gcos, Generation::g1(), x1_v, x2_v, tc.debug_flag_);

            // Things to test:
            // - deep_move_interior()   // used from MutationLogStore
            // - forward_inplace_aux()  // used from DX1Collector.visit_child

            {
                bool sanitize_flag = true;

                // swaps to- and from- spaces again
                // Now from-space will be empty, all live objects in to-space

                gcos.cleanup_phase(Generation::g1(), sanitize_flag);
            }

            {
                // traverses stored objects, updates counters
                // in verify_stats (= gco.p_verify_stats_, via ctor)
                //
                gcos.verify_ok();

                INFO(tostr(xtag("n_gc_root", fixture.verify_stats_.n_gc_root_),
                           xtag("n_ext", fixture.verify_stats_.n_ext_),
                           xtag("n_from", fixture.verify_stats_.n_from_),
                           xtag("n_to", fixture.verify_stats_.n_to_),
                           xtag("n_fwd", fixture.verify_stats_.n_fwd_),
                           xtag("n_no_iface", fixture.verify_stats_.n_no_iface_)));

                REQUIRE(fixture.verify_stats_.is_ok());
            }

            {
                obj<AGCObject> report_gco;
                bool ok = gcos.report_object_types(report_mm, error_mm, &report_gco);

                REQUIRE(ok);
                REQUIRE(report_gco);

                // TODO: print report_gco, verify output

                // discard report

                report_gco.reset();
                report_mm->clear();
            }

            {
                obj<AGCObject> report_gco;
                bool ok = gcos.report_object_ages(report_mm, error_mm, &report_gco);

                if (!ok) {
                    log1.retroactively_enable();
                    log1 && log1(xtag("error", report_mm.last_error()));
                }

                REQUIRE(ok);
                REQUIRE(report_gco);

                // TODO: print report_gco, verify output

                // discard report
                
                report_gco.reset();
                report_mm->clear();
            }
        } /* loop over test cases */
    } /* TEST_CASE(GCObjectStore-1) */

} /*namespace ut*/

/* end GCObjectStore.test.cpp */
