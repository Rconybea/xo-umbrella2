/** @file dwim.test.cpp **/

#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::begin;
    using xo::pp::split;
    using xo::pp::end;
    using std::stringstream;

    TEST_CASE("dwim-structuring", "[dwim]") {
        /* dwim dispatches structuring descriptors to begin/split/newline/end,
         * and anything else to pp().  FlatSink: begin/end no-ops, split(0,0)
         * emits its 0 spaces, so "(" 1 "," 2 ")" comes out flat.
         */
        stringstream ss;
        FlatSink sink(ss);

        sink.dwim("(");
        sink.dwim(begin(2));
        sink.dwim(1);
        sink.dwim(",");
        sink.dwim(split(0, 0));
        sink.dwim(2);
        sink.dwim(end);
        sink.dwim(")");

        REQUIRE(ss.str() == "(1,2)");
    }

    TEST_CASE("dwim-operator-call", "[dwim]") {
        /* PpSink::operator() applies dwim() to each argument left-to-right,
         * so a whole structured line is one call.
         */
        stringstream ss;
        FlatSink sink(ss);

        sink("(", begin(2), 1, ",", split(0, 0), 2, end, ")");

        REQUIRE(ss.str() == "(1,2)");
    }
} /*namespace ut*/

/* end dwim.test.cpp */
