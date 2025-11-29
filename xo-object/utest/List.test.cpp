/* @file List.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/object/List.hpp"
#include "xo/object/String.hpp"
#include "xo/object/Integer.hpp"
#include "xo/alloc/GC.hpp"
#include "xo/alloc/ArenaAlloc.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <ranges>
#include <vector>
#include <string>

namespace xo {
    namespace ut {
        using xo::obj::List;
        using xo::obj::String;
        using xo::obj::Integer;
        using xo::gc::GC;
        using xo::gc::generation_result;
        using xo::gc::generation;
        using xo::gc::ArenaAlloc;

        namespace {
            struct Testcase_List {
                Testcase_List(std::size_t nz, std::size_t tz, std::size_t ngct, std::size_t tgct,
                              const std::vector<std::vector<std::string>> & v)
                : nursery_z_{nz}, tenured_z_{tz}, incr_gc_threshold_{ngct}, full_gc_threshold_{tgct}, v_{v}
                {}

                std::size_t nursery_z_;
                std::size_t tenured_z_;
                std::size_t incr_gc_threshold_;
                std::size_t full_gc_threshold_;

                std::vector<std::vector<std::string>> v_;

                std::string expect_display_;
            };

            std::vector<Testcase_List>
            s_testcase_v = {
                Testcase_List(1024, 2048, 512, 1024, {{}}),
                Testcase_List(2048, 4096, 512, 1024, {{"hello", ", ", " world!"}}),
                Testcase_List(2048, 4096, 512, 1024, {{"the", " quick", " brown", "fox", "jumps"},
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
                     .incr_gc_threshold_ = tc.incr_gc_threshold_,
                     .full_gc_threshold_ = tc.full_gc_threshold_,
                     .debug_flag_ = c_debug_flag});
                gc->disable_gc();

                REQUIRE(gc.get());

                /* use gc for all Object allocs */
                Object::mm = gc.get();

                {
                    scope log(XO_DEBUG(c_debug_flag));
                    log && log(xtag("i_tc", i_tc), xtag("tc.v_.size", tc.v_.size()));

                    std::vector<gp<List>> root_v(tc.v_.size());
                    std::size_t i = 0;

                    /* auditing List::_shallow_size(), String::_shallow_size()
                     * vs GC::allocated()
                     */
                    std::size_t expected_alloc_z = 0;

                    // TODO: consolidate: root setup shared with "List" unit test

                    REQUIRE(gc->allocated() == expected_alloc_z);

                    /* construct example Lists from testcase info */
                    for (const std::vector<std::string> & v : tc.v_)
                    {
                        INFO(xtag("v", v));

                        /* building l1 in reverse order */
                        gp<List> l1 = List::nil;

                        for (std::size_t ip1 = v.size(); ip1 > 0; --ip1) {
                            INFO(xtag("ip1", ip1));

                            const std::string & si = v.at(ip1 - 1);

                            log && log(xtag("i", ip1-1), xtag("si", si));

                            gp<String> sobj = String::copy(si.c_str());

                            std::size_t sobj_z = sobj->_shallow_size();
                            expected_alloc_z += sobj_z;

                            REQUIRE(gc->allocated() == expected_alloc_z);

                            l1 = List::cons(sobj, l1);

                            log && log(xtag("l1.size", l1->size()));

                            std::size_t l1_z = l1->_shallow_size();
                            expected_alloc_z += l1_z;

                            REQUIRE(gc->allocated() == expected_alloc_z);
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
                    gc->enable_gc_once();

                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
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
                    gc->enable_gc_once();

                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 2);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
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

                    REQUIRE(gc->native_gc_statistics().total_promoted_ == gc->allocated());

                    gc->request_gc(generation::tenured);
                    gc->enable_gc_once();

                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 2);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 1);
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

                    log && log("stats", gc->native_gc_statistics());
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
                     .incr_gc_threshold_ = tc.incr_gc_threshold_,
                     .full_gc_threshold_ = tc.full_gc_threshold_,
                    .debug_flag_ = c_debug_flag});

                REQUIRE(gc.get());

                gc->disable_gc();

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
                    gc->enable_gc_once();

                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
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

        TEST_CASE("List.display", "[List]")
        {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 1024, c_debug_flag);

            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            gp<List> l = List::list(Integer::make(mm, 1), Integer::make(mm, 2), Integer::make(mm, 3));

            REQUIRE(l->size() == 3);

            std::stringstream ss;
            ss << l;

            REQUIRE(ss.str() == "(1 2 3)");
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end List.test.cpp */
