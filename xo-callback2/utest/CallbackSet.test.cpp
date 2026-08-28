/** @file CallbackSet.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include <xo/callback2/CallbackSet.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::fn::CallbackSet;
    using xo::fn::CallbackId;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace {
        using PlainSet = CallbackSet<void (*)(int)>;

        /* callbacks are plain function pointers, so shared state has to be
         * file-scope -- that is exactly the tradeoff the no-capture rule buys
         */
        int s_sum = 0;
        int s_count = 0;

        void cb_add(int x) { s_sum += x; ++s_count; }
        void cb_double(int x) { s_sum += 2*x; ++s_count; }

        /* for the reentrancy cases */
        PlainSet * s_set = nullptr;
        CallbackId s_victim;

        void cb_self_remove(int x) {
            s_sum += x;
            ++s_count;
            s_set->remove_callback(s_victim);
        }

        void cb_self_add(int x) {
            s_sum += x;
            ++s_count;
            s_set->add_callback(&cb_add);
        }

        void reset() { s_sum = 0; s_count = 0; s_set = nullptr; }
    }

    /* the traced/untraced split is a compile-time property */
    static_assert(PlainSet::is_traced == false);
    static_assert(CallbackSet<obj<AAllocator>>::is_traced == true);

    TEST_CASE("cbset2-empty", "[callback2]") {
        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 4*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 8);

        REQUIRE(s != nullptr);
        CHECK(s->capacity() == 8);
        CHECK(s->size() == 0);
        CHECK(s->is_empty());
        CHECK(!s->is_full());
        CHECK(!s->is_running());
    }

    TEST_CASE("cbset2-add-invoke", "[callback2]") {
        reset();

        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 4*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 8);
        REQUIRE(s);

        CallbackId id1 = s->add_callback(&cb_add);
        CallbackId id2 = s->add_callback(&cb_double);

        CHECK(id1 != id2);
        CHECK(s->size() == 2);

        s->invoke(5);

        /* 5 + 10 */
        CHECK(s_sum == 15);
        CHECK(s_count == 2);
        CHECK(!s->is_running());
    }

    TEST_CASE("cbset2-remove", "[callback2]") {
        reset();

        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 4*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 8);
        REQUIRE(s);

        CallbackId id1 = s->add_callback(&cb_add);
        s->add_callback(&cb_double);

        CHECK(s->remove_callback(id1) == true);
        CHECK(s->size() == 1);
        /* removing twice is a no-op, not an error */
        CHECK(s->remove_callback(id1) == false);

        s->invoke(5);

        CHECK(s_sum == 10);   /* only cb_double survived */
        CHECK(s_count == 1);
    }

    TEST_CASE("cbset2-full", "[callback2]") {
        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 4*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 2);
        REQUIRE(s);

        CHECK(s->add_callback(&cb_add) != CallbackId());
        CHECK(s->add_callback(&cb_add) != CallbackId());
        CHECK(s->is_full());
        /* capacity is fixed: the third add fails rather than reallocating */
        CHECK(s->add_callback(&cb_add) == CallbackId());
        CHECK(s->size() == 2);
    }

    TEST_CASE("cbset2-copy-grow", "[callback2]") {
        reset();

        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 8*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 2);
        REQUIRE(s);

        s->add_callback(&cb_add);
        s->add_callback(&cb_double);

        PlainSet * bigger = PlainSet::copy(mm, s, 8);

        REQUIRE(bigger != nullptr);
        CHECK(bigger->capacity() == 8);
        CHECK(bigger->size() == 2);
        CHECK(!bigger->is_full());

        /* callbacks survived the copy */
        bigger->invoke(5);
        CHECK(s_sum == 15);

        /* shrinking below size is refused */
        CHECK(PlainSet::copy(mm, s, 1) == nullptr);
    }

    TEST_CASE("cbset2-reentrant-remove", "[callback2]") {
        reset();

        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 4*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 8);
        REQUIRE(s);
        s_set = s;

        s->add_callback(&cb_self_remove);
        s_victim = s->add_callback(&cb_double);

        /* cb_self_remove removes cb_double WHILE the set is running.
         * The removal must be deferred, so cb_double still runs this time.
         */
        s->invoke(5);

        CHECK(s_count == 2);
        CHECK(s_sum == 15);
        CHECK(s->size() == 1);      /* deferred removal applied at end */
        CHECK(!s->is_running());

        /* second run: only cb_self_remove remains */
        reset();
        s_set = s;
        s->invoke(5);

        CHECK(s_count == 1);
        CHECK(s_sum == 5);
    }

    TEST_CASE("cbset2-reentrant-add", "[callback2]") {
        reset();

        ArenaConfig cfg { .name_ = "cbset-arena", .size_ = 4*1024 };
        DArena arena = DArena::map(cfg);
        auto mm = with_facet<AAllocator>::mkobj(&arena);

        PlainSet * s = PlainSet::_empty(mm, 8);
        REQUIRE(s);
        s_set = s;

        s->add_callback(&cb_self_add);

        /* the added callback must NOT run during the invoke that added it */
        s->invoke(5);

        CHECK(s_count == 1);
        CHECK(s->size() == 2);      /* deferred add applied at end */

        /* now both run */
        reset();
        s_set = s;
        s->invoke(5);

        CHECK(s_count == 2);
    }
} /*namespace ut*/

/* end CallbackSet.test.cpp */
