/** @file scope.test.cpp **/

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>

namespace ut {

    using xo::scope;
    using xo::print::FlatSink;
    using xo::print::ThreadLogState;
    using std::stringstream;

    TEST_CASE("scope-indent-flat", "[scope]") {
    /* capture scope output into a stringstream via a FlatSink */
    stringstream ss;
    FlatSink sink(ss);
    ThreadLogState::log_set_sink(&sink);

    {
        scope outer("outer");
        outer.log("hello");
        {
            scope inner("inner");
            inner.log("world", 42);
        }
        outer.log("bye");
    }

    ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */

    REQUIRE(ss.str() ==
            "+outer\n"
            "  hello\n"
            "  +inner\n"
            "    world42\n"
            "  -inner\n"
            "  bye\n"
            "-outer\n");
}

} /*namespace ut*/

/* end scope.test.cpp */
