/** @file span_pp.test.cpp
 *
 *  exercise xo::pp::Prettifier<xo::mm::span<CharT>> (from xo/arena/span_pp.hpp)
 *  -- the ppsink-native structured printing for an arena span.
 **/

#include <xo/arena/span_pp.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::FlatSink;
    using std::stringstream;

    static std::string pretty_of(const xo::mm::span<const char> & sp) {
        stringstream ss;
        FlatSink sink(ss);
        sink.pp(sp);
        return ss.str();
    }

    TEST_CASE("span_pp-basic", "[span_pp]") {
        const char * text = "hello";
        xo::mm::span<const char> sp(text, text + 5);

        std::string out = pretty_of(sp);

        REQUIRE(out.substr(0, 5) == "<span");
        REQUIRE(out.back() == '>');
        REQUIRE(out.find(":size 5") != std::string::npos);
        REQUIRE(out.find(":text hello") != std::string::npos);
        REQUIRE(out.find(":addr ") != std::string::npos);
    }

    TEST_CASE("span_pp-empty", "[span_pp]") {
        /* a null span: size 0, empty text */
        xo::mm::span<const char> sp = xo::mm::span<const char>::make_null();

        std::string out = pretty_of(sp);

        REQUIRE(out.find(":size 0") != std::string::npos);
        REQUIRE(out.find(":text >") != std::string::npos);   /* empty content, then '>' */
    }
} /*namespace ut*/

/* end span_pp.test.cpp */
