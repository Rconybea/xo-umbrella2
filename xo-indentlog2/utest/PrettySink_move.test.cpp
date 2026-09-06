/** @file PrettySink_move.test.cpp
 *
 *  Pin PrettySink's move constructor.
 *
 *  WHY THIS EXISTS
 *
 *  PrettySink holds two INTERIOR pointers: sbuf_ refers to pps_, and pps_
 *  refers to logbuf_ -- each a sibling member of the same PrettySink.  The
 *  compiler-generated memberwise move left both aimed at the moved-FROM
 *  object, so a moved sink either rendered EMPTY or crashed inside
 *  LogBufferAdapter::write_span (memcpy with a null destination).
 *
 *  That defect was invisible for as long as it existed, for a specific
 *  reason: nothing in c++ ever moves a PrettySink.  make2str()/make2cout()
 *  return by value, but every such return is a prvalue, so guaranteed copy
 *  elision constructs directly into the destination and the move ctor is
 *  never called.  The first caller to actually invoke it was a pybind11
 *  binding (xo-pyindentlog2), which moves a factory's return value onto the
 *  heap -- and it segfaulted.
 *
 *  So these cases assert the property no other test can: that a sink still
 *  WORKS after being moved.  A failure here means the move ctor has stopped
 *  repairing something.  If a member that points at a sibling was recently
 *  added to PrettySink, that is where to look.
 *
 *  NB the interesting case is `moved mid-render` -- the state a memberwise
 *  move most obviously corrupts is a partially-built record.
 **/

#include "print/PrettySink.hpp"
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <catch2/catch.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::field;

    namespace {
        PpConfig cfg(std::uint32_t margin = 60) {
            return PpConfig::scratch_plain(margin);
        }

        /** move @p src onto the heap, then DESTROY the source.
         *
         *  This is precisely what a pybind11 factory binding does, and the
         *  distinction matters: while the moved-from sink stays alive, a
         *  stale interior pointer still addresses a live (if hollowed-out)
         *  object and the damage can stay invisible.  Only once the source is
         *  destroyed does a missed repair become a dangling pointer.
         *
         *  Tests that keep `src` in scope do NOT exercise that, which is why
         *  the cases below hand ownership over instead.
         **/
        std::unique_ptr<PrettySink> moved_to_heap(PrettySink && src) {
            return std::make_unique<PrettySink>(std::move(src));
        }
    } /*namespace*/

    TEST_CASE("prettysink-move-preserves-output", "[PrettySink][move]") {
        /* text already written must survive the move */
        PrettySink a = PrettySink::make2str(cfg());
        a.put("hello");

        PrettySink b = std::move(a);

        REQUIRE(b.output() == "hello");
    }

    TEST_CASE("prettysink-move-still-writable", "[PrettySink][move]") {
        /* THE case that failed: before the hand-written move ctor this
         * produced "" -- sbuf_ was still writing into the moved-from pps_
         */
        PrettySink a = PrettySink::make2str(cfg());
        PrettySink b = std::move(a);

        b.put("hello");

        REQUIRE(b.output() == "hello");
    }

    TEST_CASE("prettysink-move-mid-render", "[PrettySink][move]") {
        /* move with a record half-built, then finish it.  The result must
         * match the same sequence performed without any move.
         */
        std::string unmoved;
        {
            PrettySink s = PrettySink::make2str(cfg());
            s.pretty_struct("Foo", field("a", 1), field("b", 2));
            unmoved = std::string(s.output());
        }

        PrettySink a = PrettySink::make2str(cfg());
        a.put("<Foo");
        PrettySink b = std::move(a);        /* mid-record */
        b.put(" :a 1 :b 2>");

        REQUIRE(b.output() == unmoved);
    }

    TEST_CASE("prettysink-move-then-complete", "[PrettySink][move]") {
        /* complete() drains through logbuf_ -- i.e. through the pps_ -> logbuf_
         * pointer, the second of the two the move ctor repairs.
         *
         * NB complete() RECLAIMS logbuf_, so output() is empty afterwards --
         * that is by design (see PrettySink::complete), not a move artifact.
         * Confirmed against an unmoved sink, which behaves identically.  So
         * what this pins is that a moved sink drains and then keeps working.
         */
        PrettySink a = PrettySink::make2str(cfg());
        PrettySink b = std::move(a);

        b.put("line");
        REQUIRE(b.output() == "line");      /* buffered before the drain */

        b.complete();
        REQUIRE(b.output() == "");          /* reclaimed, as when unmoved */

        /* still usable for a second record */
        b.put("next");
        REQUIRE(b.output() == "next");
    }

    TEST_CASE("prettysink-move-repeatedly", "[PrettySink][move]") {
        /* a chain of moves must be as good as none: each repairs onto the
         * newest object, so a stale pointer would surface by the last hop
         */
        PrettySink a = PrettySink::make2str(cfg());
        a.put("x");

        PrettySink b = std::move(a);
        PrettySink c = std::move(b);
        PrettySink d = std::move(c);

        d.put("y");

        REQUIRE(d.output() == "xy");
    }

    TEST_CASE("prettysink-move-line-breaking-intact", "[PrettySink][move]") {
        /* layout state (margins, indent) lives in pps_.  A move must not
         * disturb WHERE the sink decides to break -- checked by rendering a
         * record too wide for the margin and comparing against no-move.
         */
        auto render = [](bool do_move) {
            PrettySink s = PrettySink::make2str(cfg(20));
            if (do_move) {
                PrettySink t = std::move(s);
                t.pretty_struct("Wide",
                                field("alpha", 111), field("beta", 222),
                                field("gamma", 333), field("delta", 444));
                return std::string(t.output());
            }
            s.pretty_struct("Wide",
                            field("alpha", 111), field("beta", 222),
                            field("gamma", 333), field("delta", 444));
            return std::string(s.output());
        };

        std::string moved = render(true);
        std::string unmoved = render(false);

        REQUIRE(moved == unmoved);
        /* and it really did break -- otherwise this proves nothing */
        REQUIRE(moved.find('\n') != std::string::npos);
    }
    TEST_CASE("prettysink-move-with-dest-sbuf", "[PrettySink][move]") {
        /* THE case that make2str could not reach.
         *
         * With a drain destination attached, complete() goes all the way
         * through LogBufferAdapter into the arena -- via buf_v_, an interior
         * pointer from the adapter BASE to LogBuffer's own arena_ member.
         * That one is a third interior pointer, distinct from the two
         * PrettySink repairs, and it used to be a reference (unrebindable).
         * A moved sink therefore drained through the moved-FROM arena and
         * segfaulted, while the make2str path -- dest_ == nullptr, so no
         * drain -- appeared to work.
         */
        std::stringstream dest;

        /* source destroyed at the end of this full expression */
        auto b = moved_to_heap([&]{
            PrettySink s = PrettySink::make2str(cfg());
            s.set_dest_sbuf(dest.rdbuf());
            return s;
        }());

        /* the invariants, asserted directly: a stale interior pointer is UB
         * that often does not fault, so a behavioural check alone would not
         * catch it.  verify_ok covers both PrettySink repairs and LogBuffer's.
         */
        REQUIRE(b->verify_ok());

        b->put("drained");
        b->complete();                  /* used to crash here */

        REQUIRE(dest.str().substr(0, 7) == "drained");
    }

    TEST_CASE("prettysink-move-survives-many-records", "[PrettySink][move]") {
        /* repeated complete() cycles on a moved sink: each reclaims and
         * re-allocates out of the arena, so a stale buf_v_ would show up
         * quickly rather than only on the first drain
         */
        std::stringstream dest;

        auto b = moved_to_heap([&]{
            PrettySink s = PrettySink::make2str(cfg());
            s.set_dest_sbuf(dest.rdbuf());
            return s;
        }());

        for (int i = 0; i < 100; ++i) {
            b->put("rec");
            b->complete();
        }

        REQUIRE(dest.str().size() >= 300);
    }
} /*namespace ut*/

/* end PrettySink_move.test.cpp */
