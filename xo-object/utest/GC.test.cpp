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

            REQUIRE(gc->gc_statistics().n_mutation_ == 0);
            REQUIRE(gc->gc_statistics().n_logged_mutation_ == 0);

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
                REQUIRE(gc->gc_statistics().n_mutation_ == 1);
                REQUIRE(gc->gc_statistics().n_logged_mutation_ == 0);
                REQUIRE(gc->gc_statistics().n_xgen_mutation_ == 0);
                REQUIRE(gc->gc_statistics().n_xckp_mutation_ == 0);
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

                REQUIRE(gc->gc_statistics().n_mutation_ == 2);
                REQUIRE(gc->gc_statistics().n_logged_mutation_ == 1);
                REQUIRE(gc->gc_statistics().n_xgen_mutation_ == 0);
                REQUIRE(gc->gc_statistics().n_xckp_mutation_ == 1);
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

                REQUIRE(gc->gc_statistics().n_mutation_ == 2);
                REQUIRE(gc->gc_statistics().n_logged_mutation_ == 1);
                // counters recorded when mutation created.
                // not modified by gc
                REQUIRE(gc->gc_statistics().n_xgen_mutation_ == 0);
                REQUIRE(gc->gc_statistics().n_xckp_mutation_ == 1);
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
                REQUIRE(from_model.roots_.size() == to_model.nodes_.size());
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
                            throw std::runtime_error("expecting object graph containing int|cons only");

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

            struct testcase_stresstest {
                testcase_stresstest(std::size_t nz, std::size_t tz, std::size_t m, std::size_t n, std::size_t r, std::size_t k, bool debug_flag)
                    : nursery_z_{nz}, tenured_z_{tz}, m_{m}, n_{n}, r_{r}, k_{k}, debug_flag_{debug_flag}
                    {}

                std::size_t nursery_z_;
                std::size_t tenured_z_;

                /* #of random list cells to create */
                std::size_t m_;
                /* #of random integers to create */
                std::size_t n_;
                /* #of gc roots to create */
                std::size_t r_;
                /* #of random mutations */
                std::size_t k_;

                bool debug_flag_;
            };

            std::vector<testcase_stresstest> s_testcase_v =
            {
                testcase_stresstest(1024, 1024, 3, 7, 5, 10, true)
            };
        } /*namespace*/

        TEST_CASE("gc-stresstest", "[alloc][gc][gc_mutation]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const testcase_stresstest & tc = s_testcase_v[i_tc];

                scope log(XO_DEBUG(tc.debug_flag_));

                up<GC> gc = GC::make(
                    {
                        .initial_nursery_z_ = tc.nursery_z_,
                        .initial_tenured_z_ = tc.tenured_z_,
                        .debug_flag_ = tc.debug_flag_
                    });

                REQUIRE(gc->gc_statistics().n_mutation_ == 0);
                REQUIRE(gc->gc_statistics().n_logged_mutation_ == 0);

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

                /* create m random list cells */
                size_t m = tc.m_;
                /* create n random integers */
                size_t n = tc.n_;
                /* #of roots */
                size_t r = tc.r_;
                /* #of random mutations */
                size_t k = tc.k_;

                REQUIRE(m > 0);
                REQUIRE(n > 0);

                /* w1[] contains some random list cells */
                std::vector<gp<List>> w1;
                {
                    for (size_t i = 0; i < m; ++i) {
                        w1.push_back(List::cons(List::nil, List::nil));
                    }
                    REQUIRE(w1.size() == m);
                }

                /* w2[] has all of w1[], also contains some integers */
                std::vector<gp<Object>> w2;
                {
                    std::copy(w1.begin(), w1.end(), std::back_inserter(w2));
                    for (size_t j = 0; j < n; ++j) {
                        w2.push_back(Integer::make(j));
                    }
                    REQUIRE(w2.size() == m + n);
                }

                /* create some random roots. always pick at least one list cell */
                std::vector<gp<Object>> root_v;
                std::size_t w1_ix = rgen() % n;
                {
                    root_v.push_back(w2.at(w1_ix));
                    for (std::size_t i = 1; i < r; ++i) {
                        std::size_t w2_ix = rgen() % (m + n);

                        root_v.push_back(w2.at(w2_ix));
                    }

                    for (auto & root : root_v)
                        gc->add_gc_root(root.ptr_address());
                }

                /* random mutations -- these will get logged */
                {
                    for (std::size_t i = 0; i < k; ++i) {
                        /* pick a list cell at random */
                        gp<List> l1 = w1.at(rgen() % w1.size());

                        if (rgen() % 2 == 0) {
                            /* pick another list cell at random, and link it to l1 */
                            gp<List> l2 = w1.at(rgen() % w1.size());

                            l1->assign_rest(l2);
                        } else {
                            /* pick a value at random (could be list or integer),
                             * assign to head
                             */
                            gp<Object> x2 = w2.at(rgen() % w2.size());

                            l1->assign_head(x2);
                        }
                    }
                }

                log && log("stats.before", gc->gc_statistics());

                /* make model for contents of w2[] */
                ObjectGraphModel from_model;
                from_model.from_root_vector(root_v);

                gc->request_gc(generation::nursery);

                /* collector cycle changed object addresses.
                 * build a new object model, and verify that they're equivalent
                 */

                ObjectGraphModel to_model;
                to_model.from_root_vector(root_v);

                REQUIRE(ObjectGraphModel::verify_equal_models(from_model, to_model));

                log && log("stats.after", gc->gc_statistics());
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end GC.test.cpp */
