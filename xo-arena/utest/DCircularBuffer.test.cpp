/** @file DCircularBuffer.test.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DCircularBuffer.hpp"
#include "print.hpp"
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DCircularBuffer;
    using xo::mm::CircularBufferConfig;
    using xo::mm::span;
    using std::byte;

    namespace ut {
        TEST_CASE("DCircularBuffer-tiny", "[arena][DCircularBuffer]")
        {
            // buffer works with bytes, not chars

            CircularBufferConfig cfg { .name_ = "testcbuf",
                                       .max_capacity_ = 1 };
            DCircularBuffer buf = DCircularBuffer::map(cfg);

            REQUIRE(buf.reserved_range().size() == getpagesize());
            REQUIRE(buf.mapped_range().size() == 0);
            REQUIRE(buf.occupied_range().size() == 0);
            REQUIRE(buf.input_range().size() == 0);

            REQUIRE(buf.verify_ok(verify_policy::log_only()));
            REQUIRE(buf.get_append_span(1).size() == getpagesize());
            REQUIRE(buf.mapped_range().size() == getpagesize());
            REQUIRE(buf.occupied_range().size() == 0);
            REQUIRE(buf.input_range().size() == 0);

            auto s0 = DCircularBuffer::const_span_type::from_cstr("abcdefghijk");
            /* return value is unaccepted suffix of input */
            REQUIRE(buf.append(s0).empty());
            REQUIRE(buf.verify_ok(verify_policy::log_only()));
            REQUIRE(buf.mapped_range().size() == getpagesize());
            REQUIRE(buf.occupied_range().size() == s0.size());
            REQUIRE(buf.input_range().size() == s0.size());

            auto s1 = DCircularBuffer::const_span_type::from_cstr("lmnopq");
            REQUIRE(buf.append(s1).empty());
            REQUIRE(buf.mapped_range().size() == getpagesize());
            REQUIRE(buf.occupied_range().size() == s0.size() + s1.size());

            REQUIRE(buf.occupied_range().to_string_view() == std::string_view("abcdefghijklmnopq"));

            buf.consume(buf.occupied_range().prefix(3));

            REQUIRE(buf.occupied_range().to_string_view() == std::string_view("defghijklmnopq"));
        }

        // TODO: test pin_range() / unpin_range()
    } /*namespace ut*/
} /*namespace xo*/

/* end DCircularBuffer.test.cpp */
