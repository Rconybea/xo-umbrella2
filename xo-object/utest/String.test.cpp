/* @file String.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/object/String.hpp"
#include "xo/alloc/GC.hpp"
#include "xo/alloc/ArenaAlloc.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/quoted.hpp"
#include <catch2/catch.hpp>
#include <cstring>
#include <cstdint>

namespace xo {
    using xo::gc::IAlloc;
    using xo::gc::GC;
    using xo::gc::ArenaAlloc;
    using xo::gc::generation;
    using xo::obj::String;

    namespace ut {

        namespace {
            struct Testcase_String {
                Testcase_String(std::size_t nz,
                                std::size_t tz,
                                std::size_t ngct,
                                std::size_t tgct,
                                const std::vector<std::string> & v) : nursery_z_{nz},
                                                                      tenured_z_{tz},
                                                                      incr_gc_threshold_{ngct},
                                                                      full_gc_threshold_{tgct},
                                                                      v_{v} {}

                std::size_t nursery_z_;
                std::size_t tenured_z_;
                std::size_t incr_gc_threshold_;
                std::size_t full_gc_threshold_;

                std::vector<std::string> v_;
            };

            std::vector<Testcase_String>
            s_testcase_v = {
                Testcase_String(1024, 4096, 512, 512, {"hello"}),
                // in emacs: C-x 8 RET lambda
                //
                Testcase_String(1024, 4096, 512, 512, {"λ"}),
                Testcase_String(1024, 4096, 512, 512, {"hello", ", world!"})
            };
        }

        TEST_CASE("String", "[String][gc]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_String & tc = s_testcase_v[i_tc];

                up<GC> gc = GC::make(
                    {.initial_nursery_z_ = tc.nursery_z_,
                     .initial_tenured_z_ = tc.tenured_z_,
                     .incr_gc_threshold_ = tc.incr_gc_threshold_,
                     .full_gc_threshold_ = tc.full_gc_threshold_,
                    .debug_flag_ = false});

                REQUIRE(gc.get());

                gc->disable_gc();

                /* use gc for all Object allocs */
                Object::mm = gc.get();

                {
                    std::size_t n_string = 0;
                    std::size_t expected_alloc_z = 0;

                    for (const std::string & s_str : tc.v_)
                    {
                        gp<String> s1 = String::copy(s_str.c_str());

                        ++n_string;
                        /* 1+ for mandatory null terminator */
                        expected_alloc_z += (IAlloc::with_padding(sizeof(String))
                                             + IAlloc::with_padding(1 + s_str.length()));

                        REQUIRE(gc->allocated() % sizeof(std::uintptr_t) == 0);
                        REQUIRE(gc->allocated() == expected_alloc_z);

                        REQUIRE(s1->length() == s_str.length());
                        REQUIRE(strcmp(s1->c_str(), s_str.c_str()) == 0);
                    }

                    /* gc has n_string objects.  Nothing refers to them, so gc going to kill all */
                    gc->request_gc(generation::nursery);
                    gc->enable_gc_once();

                    REQUIRE(gc->gc_in_progress() == false);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);
                    REQUIRE(gc->allocated() == 0);
                    REQUIRE(gc->native_gc_statistics().total_allocated_ == expected_alloc_z);
                }
            }
        }

        TEST_CASE("String2", "[String][gc]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_String & tc = s_testcase_v[i_tc];

                up<GC> gc = GC::make(
                    {.initial_nursery_z_ = tc.nursery_z_,
                     .initial_tenured_z_ = tc.tenured_z_,
                     .incr_gc_threshold_ = tc.incr_gc_threshold_,
                     .full_gc_threshold_ = tc.full_gc_threshold_,
                    .debug_flag_ = false});

                REQUIRE(gc.get());

                /* use gc for all Object allocs */
                Object::mm = gc.get();

                {
                    scope log(XO_DEBUG(false));

                    std::size_t n_string = 0;
                    std::size_t expected_alloc_z = 0;

                    std::vector<gp<String>> sv(tc.v_.size());

                    std::size_t i = 0;

                    for (const std::string & s_str : tc.v_)
                    {
                        sv[i] = String::copy(s_str.c_str());

                        ++n_string;
                        /* 1+ for mandatory null terminator */
                        std::size_t alloc_z = (IAlloc::with_padding(sizeof(String))
                                               + IAlloc::with_padding(1 + s_str.length()));
                        expected_alloc_z += alloc_z;

                        log && log(xtag("s_str", xo::print::unq(s_str)),
                                   xtag("s_str.length", s_str.length()),
                                   xtag("alloc_z", alloc_z));
                        log && log(xtag("expected_alloc_z", expected_alloc_z));

                        gc->add_gc_root(reinterpret_cast<Object **>(sv[i].ptr_address()));

                        REQUIRE(gc->allocated() % sizeof(std::uintptr_t) == 0);
                        REQUIRE(gc->allocated() == expected_alloc_z);

                        REQUIRE(sv[i]->length() == s_str.length());
                        REQUIRE(strcmp(sv[i]->c_str(), s_str.c_str()) == 0);

                        ++i;
                    }

                    /* gc has a bunch of string objects; all are roots + should be preserved */
                    gc->request_gc(generation::nursery);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);

                    REQUIRE(gc->allocated() == expected_alloc_z);

                    for (std::size_t i = 0, n = sv.size(); i < n; ++i) {
                        REQUIRE(gc->contains(reinterpret_cast<std::byte *>(sv.at(i).ptr())));
                        REQUIRE(strcmp(sv.at(i)->c_str(), tc.v_[i].c_str()) == 0);
                    }
                }
            }
        }

        TEST_CASE("String.columns", "[String][unicode]")
        {
            const bool c_debug_flag = false;
            up<ArenaAlloc> arena = ArenaAlloc::make("testarena",
                                                    16*1024, c_debug_flag);

            Object::mm = arena.get();

            gp<String> s0 = String::copy("");

            REQUIRE(s0->columns() == 0);
            REQUIRE(s0->length() == 0);

            gp<String> s1 = String::copy("l");

            REQUIRE(s1->columns() == 1);
            REQUIRE(s1->length() == 1);

            gp<String> s2 = String::copy("λ");

            REQUIRE(s2->columns() == 1);
            /* two code units in code point */
            REQUIRE(s2->length() == 2);
        }

        TEST_CASE("String.append", "[String]")
        {
            const bool c_debug_flag = false;
            up<ArenaAlloc> arena = ArenaAlloc::make("testarena",
                                                    16*1024, c_debug_flag);

            Object::mm = arena.get();

            gp<String> s1 = String::share("the");
            gp<String> s2 = String::share(" quick");

            gp<String> s3 = String::append(s1, s2);

            REQUIRE(::strcmp(s1->c_str(), "the") == 0);
            REQUIRE(::strcmp(s2->c_str(), " quick") == 0);
            REQUIRE(s3.ptr());
            REQUIRE(s3->length() == s1->length() + s2->length());
            REQUIRE(::strcmp(s1->c_str(), "the quick"));

            {
                std::stringstream ss1;
                ss1 << s1;
                REQUIRE(ss1.str() == "\"the\"");
            }

            /* on printing, escape embedded " chars */
            {
                std::stringstream ss4;
                ss4 << String::share("\"Allo!\", he said");
                REQUIRE(ss4.str() == "\"\\\"Allo!\\\", he said\"");
            }
        }


    } /*namespace ut*/
} /*namespace xo*/

/* end String.test.cpp */
