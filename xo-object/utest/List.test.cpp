/* @file List.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/object/List.hpp"
#include "xo/object/String.hpp"
#include "xo/alloc/GC.hpp"
#include "xo/indentlog/scope.hpp"
#include <catch2/catch.hpp>
#include <ranges>
#include <vector>
#include <string>

namespace xo {
    namespace ut {
        using xo::obj::List;
        using xo::obj::String;
        using xo::gc::GC;
        using xo::gc::generation_result;
        using xo::gc::generation;

        namespace {
            struct Testcase_List {
                Testcase_List(std::size_t nz, std::size_t tz,
                              const std::vector<std::vector<std::string>> & v)
                : nursery_z_{nz}, tenured_z_{tz}, v_{v}
                {}

                std::size_t nursery_z_;
                std::size_t tenured_z_;

                std::vector<std::vector<std::string>> v_;
            };

            std::vector<Testcase_List>
            s_testcase_v = {
                Testcase_List( 512, 1024, {{}}),
                Testcase_List( 512, 1024, {{"hello", ", ", " world!"}}),
                Testcase_List(1024, 2048, {{"the", " quick", " brown", "fox", "jumps"},
                                           {"over", " the", " lazy", " dog!"}})
            };
        }

        TEST_CASE("List", "[List][gc]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_List & tc = s_testcase_v[i_tc];

                constexpr bool c_debug_flag = false;

                up<GC> gc = GC::make(
                    {.initial_nursery_z_ = tc.nursery_z_,
                     .initial_tenured_z_ = tc.tenured_z_,
                    .debug_flag_ = c_debug_flag});

                REQUIRE(gc.get());

                /* use gc for all Object allocs */
                Object::mm = gc.get();

                {
                    scope log(XO_DEBUG(c_debug_flag));
                    log && log(xtag("i_tc", i_tc), xtag("tc.v_.size", tc.v_.size()));

                    std::vector<gp<List>> root_v(tc.v_.size());
                    std::size_t i = 0;

                    std::size_t expected_alloc_z = 0;

                    // TODO: consolidate: root setup shared with "List" unit test

                    /* construct example Lists from testcase info */
                    for (const std::vector<std::string> & v : tc.v_)
                    {
                        /* building l1 in reverse order */
                        gp<List> l1 = List::nil;
                        for (std::size_t ip1 = v.size(); ip1 > 0; --ip1) {
                            const std::string & si = v.at(ip1 - 1);
                            log && log(xtag("i", ip1-1), xtag("si", si));
                            gp<String> sobj = String::copy(si.c_str());
                            l1 = List::cons(sobj, l1);
                            log && log(xtag("l1.size", l1->size()));

                            std::size_t alloc_z = l1->_shallow_size() + l1->head()->_shallow_size();
                            expected_alloc_z += alloc_z;
                        }

                        REQUIRE(l1->is_nil() == (v.size() == 0));
                        REQUIRE(l1->size() == v.size());

                        root_v[i] = l1;
                        gc->add_gc_root(reinterpret_cast<Object **>(root_v[i].ptr_address()));

                        REQUIRE(gc->allocated() % sizeof(std::uintptr_t) == 0);
                        REQUIRE(gc->allocated() == expected_alloc_z);

                        ++i;
                    }

                    /* gc responsible for a bunch of list objects;
                     * all are roots and should be preserved
                     */
                    gc->request_gc(generation::nursery);

                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
                    REQUIRE(gc->allocated() == expected_alloc_z);

                    /* verify GC preserved list structure and contents */
                    for (std::size_t i = 0, n = root_v.size(); i < n; ++i) {
                        std::size_t nj = tc.v_.at(i).size();

                        REQUIRE(root_v.at(i)->size() == nj);
                        if (!(root_v.at(i)->is_nil()))
                            REQUIRE(gc->contains(reinterpret_cast<std::byte *>(root_v.at(i).ptr())));

                        for (std::size_t j = 0; j < nj; ++j) {
                            gp<String> s = String::from(root_v.at(i)->list_ref(j));
                            REQUIRE(s.ptr());
                            REQUIRE(strcmp(s->c_str(), tc.v_.at(i).at(j).c_str()) == 0);

                            REQUIRE(gc->tospace_generation_of(reinterpret_cast<std::byte*>(s.ptr()))
                                    == generation_result::nursery);
                        }
                    }

                    /* every has survived one GC cycle. collect again should promote */
                    gc->request_gc(generation::nursery);

                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 2);
                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
                    REQUIRE(gc->allocated() == expected_alloc_z);

                    /* verify GC preserved list structure and contents */
                    for (std::size_t i = 0, n = root_v.size(); i < n; ++i) {
                        std::size_t nj = tc.v_.at(i).size();

                        REQUIRE(root_v.at(i)->size() == nj);
                        if (!(root_v.at(i)->is_nil()))
                            REQUIRE(gc->contains(reinterpret_cast<std::byte *>(root_v.at(i).ptr())));

                        for (std::size_t j = 0; j < nj; ++j) {
                            gp<String> s = String::from(root_v.at(i)->list_ref(j));
                            REQUIRE(s.ptr());
                            REQUIRE(strcmp(s->c_str(), tc.v_.at(i).at(j).c_str()) == 0);

                            REQUIRE(gc->tospace_generation_of(reinterpret_cast<std::byte*>(s.ptr()))
                                    == generation_result::tenured);
                        }
                    }

                    REQUIRE(gc->gc_statistics().total_promoted_ == gc->allocated());

                    gc->request_gc(generation::tenured);

                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 2);
                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 1);
                    REQUIRE(gc->allocated() == expected_alloc_z);

                    /* verify GC preserved list structure and contents */
                    for (std::size_t i = 0, n = root_v.size(); i < n; ++i) {
                        std::size_t nj = tc.v_.at(i).size();

                        REQUIRE(root_v.at(i)->size() == nj);
                        if (!(root_v.at(i)->is_nil()))
                            REQUIRE(gc->contains(reinterpret_cast<std::byte *>(root_v.at(i).ptr())));

                        for (std::size_t j = 0; j < nj; ++j) {
                            gp<String> s = String::from(root_v.at(i)->list_ref(j));
                            REQUIRE(s.ptr());
                            REQUIRE(strcmp(s->c_str(), tc.v_.at(i).at(j).c_str()) == 0);

                            REQUIRE(gc->tospace_generation_of(reinterpret_cast<std::byte*>(s.ptr()))
                                    == generation_result::tenured);
                        }
                    }

                    log && log("stats", gc->gc_statistics());
                }
            }
        } /*TEST_CASE(List, ..)*/

        TEST_CASE("List-cyclic", "[List][gc][cycles]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_List & tc = s_testcase_v[i_tc];

                constexpr bool c_debug_flag = false;

                up<GC> gc = GC::make(
                    {.initial_nursery_z_ = tc.nursery_z_,
                     .initial_tenured_z_ = tc.tenured_z_,
                    .debug_flag_ = c_debug_flag});

                REQUIRE(gc.get());

                /* use gc for all Object allocs */
                Object::mm = gc.get();

                {
                    scope log(XO_DEBUG(c_debug_flag));
                    log && log(xtag("i_tc", i_tc), xtag("tc.v_.size", tc.v_.size()));

                    std::vector<gp<List>> root_v(tc.v_.size());
                    std::size_t i = 0;

                    std::size_t expected_alloc_z = 0;

                    // TODO: consolidate: root setup shared with "List" unit test

                    /* construct example Lists from testcase info */
                    for (const std::vector<std::string> & v : tc.v_)
                    {
                        /* building l1 in reverse order */
                        gp<List> l1 = List::nil;
                        gp<List> last = List::nil;

                        for (std::size_t ip1 = v.size(); ip1 > 0; --ip1) {
                            const std::string & si = v.at(ip1 - 1);
                            log && log(xtag("i", ip1-1), xtag("si", si));
                            gp<String> sobj = String::copy(si.c_str());
                            l1 = List::cons(sobj, l1);
                            log && log(xtag("l1.size", l1->size()));
                            if (ip1 == v.size()) {
                                // capture last
                                last = l1;
                            }

                            std::size_t alloc_z = l1->_shallow_size() + l1->head()->_shallow_size();
                            expected_alloc_z += alloc_z;

                            // replace tail to make a cycle
                            if (last.ptr()) {
                                last->assign_rest(l1);
                            }
                        }

                        REQUIRE(l1->is_nil() == (v.size() == 0));
                        //REQUIRE(l1->size() == v.size());  // lwill loop forever

                        root_v[i] = l1;
                        gc->add_gc_root(reinterpret_cast<Object **>(root_v[i].ptr_address()));

                        REQUIRE(gc->allocated() % sizeof(std::uintptr_t) == 0);
                        REQUIRE(gc->allocated() == expected_alloc_z);

                        ++i;
                    }

                    gc->request_gc(generation::nursery);

                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
                    REQUIRE(gc->allocated() == expected_alloc_z);

                    /* verify GC preserved list structure and contents */
                    for (std::size_t i = 0, n = root_v.size(); i < n; ++i) {
                        std::size_t nj = tc.v_.at(i).size();

                        // REQUIRE(root_v.at(i)->size() == nj); // will loop forever
                        if (!(root_v.at(i)->is_nil()))
                            REQUIRE(gc->contains(reinterpret_cast<std::byte *>(root_v.at(i).ptr())));

                        for (std::size_t j = 0; j < nj; ++j) {
                            gp<String> s = String::from(root_v.at(i)->list_ref(j));
                            REQUIRE(s.ptr());
                            REQUIRE(strcmp(s->c_str(), tc.v_.at(i).at(j).c_str()) == 0);

                            REQUIRE(gc->tospace_generation_of(reinterpret_cast<std::byte*>(s.ptr()))
                                    == generation_result::nursery);
                        }

                        REQUIRE(root_v.at(i)->list_ref(nj).ptr() == root_v.at(i)->list_ref(0).ptr());
                    }
                }
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end List.test.cpp */
