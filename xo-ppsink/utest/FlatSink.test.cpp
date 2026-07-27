/* @file FlatSink.test.cpp */

#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>

using xo::pp::FlatSink;
using xo::pp::PpSink;

TEST_CASE("flatsink-put-and-noop-structure", "[flatsink]") {
    /* FlatSink writes put() text straight through; begin/split/end are no-ops */
    std::stringstream ss;
    FlatSink sink(ss);

    sink.put("hello").begin().put("a").split().put("b").end().put("!");

    REQUIRE(ss.str() == "helloab!");
}

TEST_CASE("flatsink-stream-open", "[flatsink]") {
    /* stream_open returns an inserter that writes straight to the ostream */
    std::stringstream ss;
    FlatSink sink(ss);
    {
        auto ins = sink.stream_open(64);
        ins << 42 << ' ' << "x";
        ins.finish();
    }
    REQUIRE(ss.str() == "42 x");
}
