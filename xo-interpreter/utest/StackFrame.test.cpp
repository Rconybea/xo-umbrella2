/** @file StackFrame.test.cpp **/

#include "xo/interpreter/init_interpreter.hpp"
#include "xo/interpreter/StackFrame.hpp"
#include "xo/object/Integer.hpp"
#include "xo/alloc/GC.hpp"
#include <catch2/catch.hpp>
#include <vector>
#include <cstdint>

namespace xo {
    using xo::scm::StackFrame;
    using xo::obj::Integer;
    using xo::gc::GC;
    using xo::gc::ArenaAlloc;
    using xo::gc::generation;
    using xo::gc::generation_result;
    using xo::reflect::TaggedPtr;

    namespace ut {
        static InitEvidence s_init = (InitSubsys<S_interpreter_tag>::require());

        namespace {
            struct Testcase_StackFrame {
                Testcase_StackFrame(const std::vector<std::int32_t> & contents) : contents_{contents} {}

                /* build xo::obj::Integer for each contents_[i], store in F[i] for new StackFrame F */
                std::vector<std::int32_t> contents_;
            };

            std::vector<Testcase_StackFrame>
            s_testcase_v = {
                Testcase_StackFrame({}),
                Testcase_StackFrame({}),
                Testcase_StackFrame({111}),
            };
        }

        TEST_CASE("StackFrame", "[StackFrame][interpreter]")
        {
            Subsystem::initialize_all();

            constexpr bool c_debug_flag = false;

            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                scope log(XO_DEBUG(c_debug_flag), xtag("test", "StackFrame2"), xtag("i_tc", i_tc));

                const Testcase_StackFrame & tc = s_testcase_v[i_tc];

                up<ArenaAlloc> alloc = ArenaAlloc::make("utest", 16384, c_debug_flag);
                REQUIRE(alloc.get());
                Object::mm = alloc.get();

                std::size_t n = tc.contents_.size();
                gp<StackFrame> frame = StackFrame::make(alloc.get(), n);

                TaggedPtr tp = frame->self_tp();

                REQUIRE(tp.is_struct());
            }
        }

        TEST_CASE("StackFrame2", "[StackFrame][gc][interpreter]")
        {
            Subsystem::initialize_all();

            constexpr bool c_debug_flag = false;

            try {
                for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                    scope log(XO_DEBUG(c_debug_flag), xtag("test", "StackFrame2"), xtag("i_tc", i_tc));

                    const Testcase_StackFrame & tc = s_testcase_v[i_tc];

                    up<GC> gc = GC::make(
                        {.initial_nursery_z_ = 16384,
                         .initial_tenured_z_ = 32768,
                         .incr_gc_threshold_ = 4096,
                         .full_gc_threshold_ = 4096,
                         .object_stats_flag_ = true,
                         .debug_flag_ = c_debug_flag,
                        });

                    REQUIRE(gc.get());

                    /* use gc for all Object allocs */
                    GC * mm = gc.get();
                    Object::mm = mm;

                    std::size_t n = tc.contents_.size();

                    gp<Integer> x = Integer::make(gc.get(), 42);
                    gc->add_gc_root(reinterpret_cast<Object **>(&x));
                    REQUIRE(gc->tospace_generation_of(x.ptr()) == generation_result::nursery);

                    gp<StackFrame> frame = StackFrame::make(gc.get(), n);
                    StackFrame ** frame_pp = frame.ptr_address();
                    gc->add_gc_root(reinterpret_cast<Object **>(frame_pp));

                    /* verifying allocated in N1 */
                    REQUIRE(gc->tospace_generation_of(frame.ptr()) == generation_result::nursery);

                    for (std::size_t i = 0; i < n; ++i)
                        (*frame)[i] = Integer::make(mm, tc.contents_.at(i));

                    std::size_t expected_alloc_z = frame->_shallow_size();
                    REQUIRE(expected_alloc_z >= sizeof(StackFrame) + n * sizeof(gp<Object>));

                    gc->request_gc(generation::nursery);  // <<<<<<<<< GC here <<<<<<<<<

                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::nursery)].n_gc_ == 1);
                    REQUIRE(gc->native_gc_statistics().gen_v_[gen2int(generation::tenured)].n_gc_ == 0);

                    /* verify Integer x preserved across gc */
                    REQUIRE(gc->tospace_generation_of(x.ptr()) == generation_result::nursery);

                    /* verify StackFrame preserved across gc */
                    REQUIRE(gc->tospace_generation_of(frame.ptr()) == generation_result::nursery);
                    REQUIRE(frame->size() == n);
                    for (std::size_t i = 0; i < n; ++i) {
                        //REQUIRE(Integer::from(frame->lookup(i)).ptr());
                        //REQUIRE(Integer::from(frame->lookup(i))->value() == tc.contents_.at(i));
                    }
                }
            } catch (std::exception & ex) {
                std::cerr << "exception: " << ex.what() << std::endl;
                REQUIRE(false);
            }
        }
    }
}

/* end StackFrame.test.cpp */
