/* @file reader.test.cpp */

#include "xo/reader/reader.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::reader;

    namespace ut {
        TEST_CASE("reader", "[reader]") {
            for (std::size_t i_tc = 0; i_tc < 1; ++i_tc) {
                reader rdr;

                constexpr bool c_debug_flag = true;
                scope log(XO_DEBUG(c_debug_flag),
                          xtag("utest", "reader"), xtag("i_tc", i_tc));

                rdr.begin_translation_unit();

                try {
                    auto rr = rdr.read_expr(reader::span_type::from_cstr("def foo : f64 = 3.14159265"),
                                            true /*eof*/);

                    REQUIRE(rr.expr_.get());
                    REQUIRE(rr.rem_.empty());
                } catch (std::exception & ex) {
                    log && log(ex.what());

                    INFO(ex.what());

                    REQUIRE(false);
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end reader.test.cpp */
