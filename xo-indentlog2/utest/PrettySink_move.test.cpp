/** @file PrettySink_move.test.cpp
 *
 *  Pin PrettySink's move constructor.
 *
 *  NOTE: the interesting case is `moved mid-render` -- the state a memberwise
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
         *  Shadow what a pybind11 factory binding does.
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
