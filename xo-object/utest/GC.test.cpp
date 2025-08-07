/* @file GC.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/GC.hpp"
#include "xo/object/List.hpp"
#include "xo/object/Integer.hpp"
#include "xo/randomgen/random_seed.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <unordered_set>

namespace xo {
    using xo::obj::List;
    using xo::obj::Integer;
    using xo::gc::GC;
    using xo::gc::generation_result;
    using xo::gc::generation;

    using xo::rng::Seed;
    using xo::rng::xoshiro256ss;

    namespace ut {

        // Also see GC unit tests in xo-alloc/utest

#ifdef NOT_YET
        namespace {
            struct testcase_mlog {
                testcase_mlog(std::size_t nz, std::size_t tz) : nursery_z_{nz}, tenured_z_{tz} {}

                std::size_t nursery_z_;
                std::size_t tenured_z_;
            };
        }
#endif

        TEST_CASE("gc-mlog-1", "[alloc][gc][gc_mutation]")
        {
            up<GC> gc = GC::make(
                {
                    .initial_nursery_z_ = 1024,
                    .initial_tenured_z_ = 2048,
                    .debug_flag_ = false
                });

            REQUIRE(gc->native_gc_statistics().n_mutation_ == 0);
            REQUIRE(gc->native_gc_statistics().n_logged_mutation_ == 0);

            REQUIRE(gc.get());

            /* use gc for all Object allocs */
            Object::mm = gc.get();

            gp<List> l = List::list(Integer::make(1));
            gc->add_gc_root(reinterpret_cast<Object**>(l.ptr_address()));

            gp<List> l2 = List::list(Integer::make(10));
            gc->add_gc_root(reinterpret_cast<Object**>(l2.ptr_address()));

            {
                REQUIRE(l->size() == 1);
                REQUIRE(l2->size() == 1);

                REQUIRE(gc->tospace_generation_of(l.ptr()) == generation_result::nursery);
                REQUIRE(gc->tospace_generation_of(l->head().ptr()) == generation_result::nursery);
                REQUIRE(gc->is_before_checkpoint(l.ptr()) == false);
                REQUIRE(gc->is_before_checkpoint(l->head().ptr()) == false);

                REQUIRE(gc->tospace_generation_of(l2.ptr()) == generation_result::nursery);
                REQUIRE(gc->tospace_generation_of(l2->head().ptr()) == generation_result::nursery);
                REQUIRE(gc->is_before_checkpoint(l2.ptr()) == false);
                REQUIRE(gc->is_before_checkpoint(l2->head().ptr()) == false);

                REQUIRE(gc->mlog_size() == 0);
            }

            // mutation, but not {xgen, xckp} since parent,child both in N0

            l->assign_head(Integer::make(2));
            {
                REQUIRE(gc->native_gc_statistics().n_mutation_ == 1);
                REQUIRE(gc->native_gc_statistics().n_logged_mutation_ == 0);
                REQUIRE(gc->native_gc_statistics().n_xgen_mutation_ == 0);
                REQUIRE(gc->native_gc_statistics().n_xckp_mutation_ == 0);
                REQUIRE(gc->mlog_size() == 0);

                REQUIRE(gc->is_gc_enabled() == true);

            }

            gc->request_gc(generation::nursery);
            {
                REQUIRE(gc->is_before_checkpoint(l.ptr()) == true);
                REQUIRE(gc->is_before_checkpoint(l->head().ptr()) == true);

                REQUIRE(gc->tospace_generation_of(l.ptr()) == generation_result::nursery);

                REQUIRE(l->size() == 1);
                REQUIRE(Integer::from(l->head()).ptr());
                REQUIRE(Integer::from(l->head())->value() == 2);
            }

            // mutation, xckp since parent in N1, child in N0

            l->assign_head(Integer::make(3));
            {
                REQUIRE(Integer::from(l->head())->value() == 3);

                REQUIRE(gc->tospace_generation_of(l->head().ptr()) == generation_result::nursery);
                REQUIRE(gc->is_before_checkpoint(l->head().ptr()) == false);

                REQUIRE(gc->native_gc_statistics().n_mutation_ == 2);
                REQUIRE(gc->native_gc_statistics().n_logged_mutation_ == 1);
                REQUIRE(gc->native_gc_statistics().n_xgen_mutation_ == 0);
                REQUIRE(gc->native_gc_statistics().n_xckp_mutation_ == 1);
                REQUIRE(gc->mlog_size() == 1);
            }

            // gc promotes parent, still need mutation log for xgen ptr

            gc->request_gc(generation::nursery);
            {
                REQUIRE(l->size() == 1);
                REQUIRE(Integer::from(l->head()).ptr());
                REQUIRE(Integer::from(l->head())->value() == 3);

                REQUIRE(gc->tospace_generation_of(l.ptr()) == generation_result::tenured);
                REQUIRE(gc->tospace_generation_of(l->head().ptr()) == generation_result::nursery);
                REQUIRE(gc->is_before_checkpoint(l->head().ptr()));

                REQUIRE(gc->native_gc_statistics().n_mutation_ == 2);
                REQUIRE(gc->native_gc_statistics().n_logged_mutation_ == 1);
                // counters recorded when mutation created.
                // not modified by gc
                REQUIRE(gc->native_gc_statistics().n_xgen_mutation_ == 0);
                REQUIRE(gc->native_gc_statistics().n_xckp_mutation_ == 1);
                REQUIRE(gc->mlog_size() == 1);
            }

            // gc promotes child, no longer need mutation log entry

            gc->request_gc(generation::nursery);
            {
                REQUIRE(l->size() == 1);
                REQUIRE(Integer::from(l->head()).ptr());
                REQUIRE(Integer::from(l->head())->value() == 3);

                REQUIRE(gc->tospace_generation_of(l.ptr()) == generation_result::tenured);
                REQUIRE(gc->tospace_generation_of(l->head().ptr()) == generation_result::tenured);

                REQUIRE(gc->mlog_size() == 0);
            }
        }

        namespace {
            enum class object_type {
                nil,
                integer,
                cons,
            };

            struct ObjectModel {
                /* 1:1 with address */
                std::size_t index_;
                /* nil|integer|cons */
                object_type type_;
                /* value for model of Integer::value_, if type_ is object_type::integer */
                std::size_t int_value_;
                /* index# for model of List::head_, if type_ is object_type::list */
                std::size_t head_ix_;
                /* index# for model of List::rest_, if type_ is object_type::list */
                std::size_t rest_ix_;
            };

            struct ObjectGraphModel {
                /**
                 *  @param from_graph
                 *  @param from_ix
                 *  @param to_graph
                 *  @param to_ix
                 *  @param p_visited_set
                 **/
                static bool verify_equal_aux(ObjectGraphModel & from_graph,
                                             std::size_t from_ix,
                                             ObjectGraphModel & to_graph,
                                             std::size_t to_ix,
                                             std::unordered_set<std::uintptr_t> * p_visited_set);

                /** compare models for structural equivalence;  will be comparing before/after a garbage collection cycle
                 *  @param from_graph     model before GC
                 *  @param to_graph       model after GC
                 *  @return true iff models are equivalent
                 **/
                static bool verify_equal_models(ObjectGraphModel & from_model, ObjectGraphModel & to_model);

                /* build model for object graph from a vector of object pointers */
                void from_root_vector(const std::vector<gp<Object>> & object_v);
                /* include everything reachable from @p x in this object model */
                std::size_t traverse_from_object(gp<Object> x);

                /* one node per xo::Object instance. */
                std::vector<ObjectModel> nodes_;
                /* map from root index to node index number */
                std::vector<std::size_t> roots_;
                /* map from (original) address to index number */
                std::unordered_map<std::uintptr_t, std::size_t> addr2node_map_;
            };

            bool
            ObjectGraphModel::verify_equal_aux(ObjectGraphModel & from_graph,
                                               std::size_t from_ix,
                                               ObjectGraphModel & to_graph,
                                               std::size_t to_ix,
                                               std::unordered_set<std::size_t> * p_visited_set)
            {
                if (p_visited_set->contains(from_ix))
                    return true;

                const ObjectModel & from = from_graph.nodes_.at(from_ix);
                const ObjectModel &   to = from_graph.nodes_.at(to_ix);

                REQUIRE(from.type_ == to.type_);

                p_visited_set->insert(from.index_);

                if (from.type_ == object_type::nil)
                    return true;

                if (from.type_ == object_type::integer) {
                    REQUIRE(from.int_value_ == to.int_value_);
                    return true;
                }

                if (from.type_ == object_type::cons) {
                    return (verify_equal_aux(from_graph, from.head_ix_,
                                             to_graph, to.head_ix_,
                                             p_visited_set)
                            && verify_equal_aux(from_graph, from.rest_ix_,
                                                to_graph, to.rest_ix_,
                                                p_visited_set));
                }

                return false;
            }

            bool
            ObjectGraphModel::verify_equal_models(ObjectGraphModel & from_model,
                                                  ObjectGraphModel &   to_model)
            {
                REQUIRE(from_model.roots_.size() == to_model.roots_.size());
                REQUIRE(from_model.nodes_.size() == to_model.nodes_.size());

                std::unordered_set<std::uintptr_t> visited_set;

                for (std::size_t i = 0, n = from_model.roots_.size(); i < n; ++i) {
                    INFO(tostr(xtag("i", i), xtag("n", n)));

                    REQUIRE(verify_equal_aux(from_model,
                                             from_model.roots_.at(i),
                                             to_model,
                                             to_model.roots_.at(i),
                                             &visited_set));
                }

                return true;
            }

            std::size_t
            ObjectGraphModel::traverse_from_object(gp<Object> x)
            {
                std::uintptr_t x_addr = reinterpret_cast<std::uintptr_t>(x.ptr());

                auto addr2node_ix = addr2node_map_.find(x_addr);

                if (addr2node_ix != addr2node_map_.end()) {
                    /* already imported (or import on call stack) */

                    return addr2node_ix->second;
                } else {
                    ObjectModel new_model;
                    auto x_int = Integer::from(x);
                    auto x_list = List::from(x);

                    std::size_t new_index = this->nodes_.size();
                    {
                        if (x_int.is_null() && x_list.is_null())
                            throw std::runtime_error(tostr("expecting object graph containing int|cons|nil only", xtag("x", x)));

                        if (!x_int.is_null()) {
                            new_model.index_ = new_index;
                            new_model.type_ = object_type::integer;
                            new_model.int_value_ = x_int->value();
                            new_model.head_ix_ = 0;
                            new_model.rest_ix_ = 0;
                        }

                        if (!x_list.is_null()) {

                            if (x_list->is_nil()) {
                                new_model.index_ = 0;
                                new_model.type_ = object_type::nil;
                                new_model.int_value_ = 0;
                                new_model.head_ix_ = 0;
                                new_model.rest_ix_ = 0;
                            } else {
                                new_model.index_ = new_index;
                                new_model.type_ = object_type::cons;
                                new_model.int_value_ = 0;
                                /* fill below */
                                new_model.head_ix_ = 0;
                                new_model.rest_ix_ = 0;
                            }
                        }
                    }

                    this->nodes_.push_back(new_model);
                    this->addr2node_map_[x_addr] = new_index;

                    if (!x_list.is_null() && !(x_list->is_nil())) {
                        ObjectModel & model = this->nodes_.at(new_index);

                        model.head_ix_ = traverse_from_object(x_list->head());
                        model.rest_ix_ = traverse_from_object(x_list->rest());
                    }

                    return new_index;
                }
            }

            void
            ObjectGraphModel::from_root_vector(const std::vector<gp<Object>> & root_v)
            {
                assert(nodes_.empty());
                assert(addr2node_map_.empty());

                /* sentinel = List::nil */
                {
                    ObjectModel sentinel;
                    sentinel.index_ = 0;
                    sentinel.type_ = object_type::nil;
                    sentinel.int_value_ = 0;
                    sentinel.head_ix_ = 0;
                    sentinel.rest_ix_ = 0;

                    this->nodes_.push_back(sentinel);
                }

                /* it's possible that object_v is complete.
                 * seed model by importing all the nodes in object_v[]
                 */
                for (gp<Object> x : root_v)
                    this->roots_.push_back(traverse_from_object(x));
            }

            /** Generate some random data + mutations to verify GC behavior
             *
             *  To setup for first GC:
             *    RandomMutationModel model(m, n, r, k);
             *    model.generate_seed_values();
             *    model.generate_random_roots(gc, &rgen);
             *    model.generate_random_mutations(&rgen);
             *
             *  To prepare for next GC
             *    model.rejuvenate_seed_values();
             *    model.alter_random_roots(&rgen);
             *    model.generate_random_mutations(&rgen);
             **/
            struct RandomMutationModel {
                RandomMutationModel(std::size_t m, std::size_t n, std::size_t r, std::size_t rr, std::size_t k)
                    : m_{m}, n_{n}, r_{r}, rr_{rr}, k_{k} {}

                void generate_seed_values();
                void generate_random_roots(GC * gc, xoshiro256ss * p_rgen);
                void generate_random_mutations(xoshiro256ss * p_rgen);

                void rejuvenate_seed_values();
                void alter_random_roots(xoshiro256ss * p_rgen);

                /* create m random list cells */
                size_t m_ = 0;
                /** create n random integers, starting with value @ref start_ **/
                size_t start_ = 0;
                size_t n_ = 0;
                /* #of roots */
                size_t r_ = 0;
                size_t rr_ = 0;
                /* #of random mutations */
                size_t k_ = 0;

                /* w1[] contains some random list cells */
                std::vector<gp<List>> w1_;
                /* w2[] has all of w1[], also contains some integers */
                std::vector<gp<Object>> w2_;

                /* create some random roots. always pick at least one list cell */
                std::vector<gp<Object>> root_v_;
            };

            void RandomMutationModel::generate_seed_values()
            {
                w1_.clear();
                w2_.clear();

                {
                    for (size_t i = 0; i < m_; ++i) {
                        w1_.push_back(List::cons(List::nil, List::nil));
                    }
                    REQUIRE(w1_.size() == m_);
                }

                {
                    std::copy(w1_.begin(), w1_.end(), std::back_inserter(w2_));
                    for (size_t j = 0; j < n_; ++j) {
                        w2_.push_back(Integer::make((this->start_)++));
                    }
                    REQUIRE(w2_.size() == m_ + n_);
                }
            }

            void RandomMutationModel::generate_random_roots(GC * gc,
                                                            xoshiro256ss * p_rgen)
            {
                std::size_t w1_ix = (*p_rgen)() % n_;
                {
                    root_v_.push_back(w2_.at(w1_ix));
                    for (std::size_t i = 1; i < r_; ++i) {
                        std::size_t w2_ix = (*p_rgen)() % (m_ + n_);

                        root_v_.push_back(w2_.at(w2_ix));
                    }

                    REQUIRE(root_v_.size() == r_);

                    for (auto & root : root_v_)
                        gc->add_gc_root(root.ptr_address());
                }

            }

            void RandomMutationModel::generate_random_mutations(xoshiro256ss * p_rgen)
            {
                for (std::size_t i = 0; i < k_; ++i) {
                    /* pick a root list cell at random */
                    gp<List> l1 = w1_.at((*p_rgen)() % w1_.size());
                    REQUIRE(l1.ptr());

                    if ((*p_rgen)() % 2 == 0) {
                        /* pick another root list cell at random, and link it to l1 */
                        gp<List> l2 = w1_.at((*p_rgen)() % w1_.size());
                        REQUIRE(l2.ptr());

                        l1->assign_rest(l2);
                    } else {
                        /* pick a value at random (could be list or integer),
                         * assign to head
                         */
                        gp<Object> x2 = w2_.at((*p_rgen)() % w2_.size());
                        REQUIRE(x2.ptr());

                        l1->assign_head(x2);
                    }
                }
            }

            void RandomMutationModel::rejuvenate_seed_values()
            {
                for (std::size_t i = 0; i < w1_.size(); ++i) {
                    INFO(xtag("i", i));

                    if (w1_.at(i)->_is_forwarded()) {
                        /* w[i] survived GC */
                        w1_[i] = dynamic_cast<List *>(w1_[i]->_destination());
                    } else {
                        /* w[i] is garbage, replace */
                        w1_[i] = List::cons(List::nil, List::nil);
                    }
                    REQUIRE(w1_[i].ptr());
                }

                for (std::size_t j = 0; j < w2_.size(); ++j) {
                    INFO(xtag("j", j));

                    if (w2_.at(j)->_is_forwarded()) {
                        /* w2[i] survived GC */
                        w2_[j] = w2_[j]->_destination();
                        REQUIRE(w2_[j].ptr());
                    } else {
                        /* w2[j] is garbage, replace */
                        w2_[j] = Integer::make((this->start_)++);
                        REQUIRE(w2_[j].ptr());
                    }
                }
            }

            void RandomMutationModel::alter_random_roots(xoshiro256ss * p_rgen)
            {
                /* replace a root value rr times */
                for (std::size_t i = 0; i < rr_; ++i) {
                    /* choose new root value at random */
                    gp<Object> new_root;
                    {
                        std::size_t j = (*p_rgen)() % (w1_.size() + w2_.size());

                        if (j < w1_.size())
                            new_root = w1_.at(j);
                        else
                            new_root = w2_.at(j - w1_.size());
                    }

                    /* choose a root to replace at random */
                    std::size_t j = (*p_rgen)() % root_v_.size();

                    root_v_[j] = new_root;
                }
            }

            struct testcase_stresstest {
                testcase_stresstest(std::size_t nz, std::size_t tz,
                                    std::size_t m, std::size_t n,
                                    std::size_t r, std::size_t rr, std::size_t k,
                                    bool gc_stats_flag, bool debug_flag)
                    : nursery_z_{nz}, tenured_z_{tz}, m_{m}, n_{n}, r_{r}, rr_{rr}, k_{k},
                      gc_stats_flag_{gc_stats_flag}, debug_flag_{debug_flag}
                    {}

                std::size_t nursery_z_;
                std::size_t tenured_z_;

                /* #of random list cells to create */
                std::size_t m_;
                /* #of random integers to create */
                std::size_t n_;
                /* #of gc roots to create */
                std::size_t r_;
                /* #of gc roots to replace between cycles */
                std::size_t rr_;
                /* #of random mutations */
                std::size_t k_;

                bool gc_stats_flag_ = false;
                bool debug_flag_ = false;
            };

            std::vector<testcase_stresstest> s_testcase_v =
            {
                /* nz: nursery size
                 * tz: tenured size
                 *  m: #of random list cells to create
                 *  n: #of random integers to create
                 *  r: #of gc roots to create
                 * rr: #of gc roots to replace between iterations
                 *  k: #of random mutations to apply
                 *
                 *                    nz    tz   m   n   r  rr   k  stats, debug */
                testcase_stresstest(1024, 1024,  5,  1,  5,  2, 10, true,  false),
                testcase_stresstest(1024, 1024, 10, 10,  5,  2, 10, true,  false)
            };
        } /*namespace*/

        TEST_CASE("gc-stresstest", "[alloc][gc][gc_mutation]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase_stresstest & tc = s_testcase_v[i_tc];

                scope log(XO_DEBUG(tc.gc_stats_flag_));

                up<GC> gc = GC::make(
                    {
                        .initial_nursery_z_ = tc.nursery_z_,
                        .initial_tenured_z_ = tc.tenured_z_,
                        .stats_flag_ = tc.gc_stats_flag_,
                        .debug_flag_ = tc.debug_flag_
                    });

                REQUIRE(gc->native_gc_statistics().n_mutation_ == 0);
                REQUIRE(gc->native_gc_statistics().n_logged_mutation_ == 0);

                REQUIRE(gc.get());

                /* use gc for all Object allocs */
                Object::mm = gc.get();

                // Plan:
                // - create vector of m cons cells w1[].
                // - prepend w1[] to a vector of n integers; call this w2[].
                // - create vector root_v[] of r  gc roots.  Assign each root_v[j] to some random w2[i]
                // - make some random mutations.
                // - traverse root_v[] to construct model from_model for reachable objects
                // - run gc
                // - traverse root_v[] again, to construct to_model for eachable objects
                // - verify from_model ~=~ to_model

                uint64_t seed = 8365237040761243362UL;
                //Seed<xoshiro256ss> seed; // to seed from /dev/random
                //std::cerr << "seed=" << seed << std::endl;
                auto rgen = xoshiro256ss(seed);

                REQUIRE(tc.m_ > 0);
                REQUIRE(tc.n_ > 0);
                REQUIRE(tc.r_ > 0);

                RandomMutationModel data_model(tc.m_, tc.n_, tc.r_, tc.rr_, tc.k_);

                for (std::size_t cycle = 0; cycle < 5; ++cycle) {
                    INFO(xtag("cycle", cycle));

                    if (cycle == 0) {
                        data_model.generate_seed_values();
                        data_model.generate_random_roots(gc.get(), &rgen);
                    } else {
                        /* figure out values in {data_model_.w1_, data_model_.w2_} that
                         * survived GC;  keep these.  Discard the remainder.
                         * don't want these as roots, because that would alter the behavior of GC.
                         *
                         * (For example want to verify behavior of GC w.r.t. cells that are alive only
                         * because of a mutation)
                         */
                        data_model.rejuvenate_seed_values();
                        data_model.alter_random_roots(&rgen);
                    }

                    data_model.generate_random_mutations(&rgen);

                    log && log(xtag("cycle", cycle),
                               xtag("stats.before", gc->get_gc_statistics()));

                    /* make model for contents of w2[] - baseline for post-GC comparison */
                    ObjectGraphModel from_model;
                    from_model.from_root_vector(data_model.root_v_);

                    gc->request_gc(generation::nursery);

                    /* collector cycle changed object addresses.
                     * build a new object model, and verify consistency with from_model
                     */
                    ObjectGraphModel to_model;
                    to_model.from_root_vector(data_model.root_v_);

                    REQUIRE(ObjectGraphModel::verify_equal_models(from_model, to_model));

                    log && log(xtag("cycle", cycle),
                               xtag("stats.after", gc->get_gc_statistics()));
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end GC.test.cpp */
