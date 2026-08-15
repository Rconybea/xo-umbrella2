/** @file pretty_struct.test.cpp
 *
 *  exercise PpSink::pretty_struct + field() from xo/ppsink/pretty_struct.hpp.
 *
 *  Two kinds of assertion here:
 *
 *  - via FlatSink: the flat *text*, which is what a reader sees when the
 *    struct fits on one line.
 *  - via MarkSink (MarkSink.hpp): the *token stream*, which FlatSink
 *    deliberately discards.  See
 *    xo-indentlog2/utest/pretty_struct.test.cpp for the rendered form.
 **/

#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp>
#include "MarkSink.hpp"
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::field;
    using xo::pp::tag;
    using std::stringstream;

    namespace {
        /** counts copies, to pin that field() does not copy its value **/
        struct Watched {
            Watched() = default;
            Watched(const Watched &) { ++n_copy_; }
            Watched & operator=(const Watched &) { ++n_copy_; return *this; }
            static int n_copy_;
        };
        int Watched::n_copy_ = 0;

        inline std::ostream & operator<<(std::ostream & os, const Watched &) {
            os << "W"; return os;
        }
    } /*namespace*/

    TEST_CASE("pretty_struct-flat", "[pretty_struct]") {
        stringstream ss;
        FlatSink sink(ss.rdbuf());

        int a = 1, b = 2;
        sink.pretty_struct("P", field("a", a), field("b", b));
        sink.complete();

        /* note the space before the FIRST field: pretty_struct emits split(1)
         * ahead of every field, matching legacy, so the name never abuts.
         */
        REQUIRE(ss.str() == "<P :a 1 :b 2>\n");
    }

    TEST_CASE("pretty_struct-no-fields", "[pretty_struct]") {
        stringstream ss;
        FlatSink sink(ss.rdbuf());

        sink.pretty_struct("P");
        sink.complete();

        REQUIRE(ss.str() == "<P>\n");
    }

    TEST_CASE("pretty_struct-token-stream", "[pretty_struct]") {
        /* the anti-flattening test.  Fails if begin() is dropped, if put(name)
         * moves inside the group, if a split between fields goes missing, or
         * if ">" escapes the group.
         */
        stringstream ss;
        MarkSink sink(ss);

        int a = 1, b = 2;
        sink.pretty_struct("P", field("a", a), field("b", b));

        REQUIRE(ss.str() ==
                "<P"
                "<G>"
                  "<S 1,0>" "<G 0>:a<S 1,1>1</G>"
                  "<S 1,0>" "<G 0>:b<S 1,1>2</G>"
                ">"
                "</G>");
    }

    TEST_CASE("pretty_struct-begin-takes-indent-from-sink", "[pretty_struct]") {
        /* "<G>" not "<G 2>": pretty_struct must call the no-arg begin(), so
         * the indent comes from the sink's PpConfig and accumulates when
         * structs nest.  Passing an explicit offset here AND in split() is the
         * double-count bug.
         */
        stringstream ss;
        MarkSink sink(ss);

        int a = 1;
        sink.pretty_struct("P", field("a", a));

        REQUIRE(ss.str().find("<G>") != std::string::npos);
        REQUIRE(ss.str().find("<G 2>") == std::string::npos);
    }

    TEST_CASE("pretty_struct-absent-field-skipped", "[pretty_struct]") {
        stringstream ss;
        FlatSink sink(ss.rdbuf());

        int a = 1, b = 2;
        sink.pretty_struct("P", field("a", a), field("b", b, false));
        sink.complete();

        REQUIRE(ss.str() == "<P :a 1>\n");
    }

    TEST_CASE("pretty_struct-absent-field-drops-separator", "[pretty_struct]") {
        /* an absent field must take its split with it -- otherwise the group
         * carries a break opportunity that renders as a stray blank column.
         */
        stringstream ss;
        MarkSink sink(ss);

        int a = 1, b = 2;
        sink.pretty_struct("P", field("a", a), field("b", b, false));

        std::string s = ss.str();
        /* exactly one separator, for the one present field */
        std::size_t n = 0;
        for (std::size_t i = s.find("<S 1,0>"); i != std::string::npos;
             i = s.find("<S 1,0>", i + 1))
        {
            ++n;
        }
        REQUIRE(n == 1);
    }

    TEST_CASE("pretty_struct-all-fields-absent", "[pretty_struct]") {
        stringstream ss;
        FlatSink sink(ss.rdbuf());

        int a = 1;
        sink.pretty_struct("P", field("a", a, false));
        sink.complete();

        REQUIRE(ss.str() == "<P>\n");
    }

    TEST_CASE("pretty_struct-tag-usable-as-field", "[pretty_struct]") {
        /* anything renderable is a field; present() is only consulted when the
         * field type has one.  tag() copies, which is fine for a small value.
         */
        stringstream ss;
        FlatSink sink(ss.rdbuf());

        sink.pretty_struct("P", tag("a", 1));
        sink.complete();

        REQUIRE(ss.str() == "<P :a 1>\n");
    }

    TEST_CASE("pretty_struct-field-does-not-copy", "[pretty_struct]") {
        /* the reason field() exists alongside tag(): a struct printer names
         * members, and copying them on every print would be a real cost for a
         * std::string member.
         */
        stringstream ss;
        FlatSink sink(ss.rdbuf());

        Watched w;
        Watched::n_copy_ = 0;

        sink.pretty_struct("P", field("w", w));
        sink.complete();

        REQUIRE(Watched::n_copy_ == 0);
        REQUIRE(ss.str() == "<P :w W>\n");
    }
} /*namespace ut*/

/* end pretty_struct.test.cpp */
