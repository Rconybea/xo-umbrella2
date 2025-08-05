/* @file GC.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/GC.hpp"
#include "xo/object/List.hpp"
#include "xo/object/Integer.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::obj::List;
    using xo::obj::Integer;
    using xo::gc::GC;
    using xo::gc::generation_result;
    using xo::gc::generation;

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
                    .debug_flag_ = true
                });

            REQUIRE(gc->gc_statistics().n_mutation_ == 0);
            REQUIRE(gc->gc_statistics().n_logged_mutation_ == 0);

            REQUIRE(gc.get());

            /* use gc for all Object allocs */
            Object::mm = gc.get();

            gp<List> l = List::list(Integer::make(1));
            gc->add_gc_root(reinterpret_cast<Object**>(l.ptr_address()));
            {
                REQUIRE(l->size() == 1);

                REQUIRE(gc->tospace_generation_of(l.ptr()) == generation_result::nursery);
                REQUIRE(gc->tospace_generation_of(l->head().ptr()) == generation_result::nursery);

                REQUIRE(gc->is_before_checkpoint(l.ptr()) == false);
                REQUIRE(gc->is_before_checkpoint(l->head().ptr()) == false);

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
    } /*namespace ut*/
} /*namespace xo*/

/* end GC.test.cpp */
