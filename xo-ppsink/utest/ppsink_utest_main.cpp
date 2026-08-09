/* @file ppsink_utest_main.cpp */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <xo/ppsink/PpStyle.hpp>

namespace {
    /** Unit tests pin rendered TEXT, so they must not be handed color escapes.
     *
     *  PpStyle's defaults are the LEGACY ones -- grey tag names, yellow struct
     *  field names (xo/ppsink/PpStyle.hpp) -- which is right for a terminal and
     *  useless in an expectation string.  Declared once here rather than in
     *  each TEST_CASE: this is a property of the whole binary.
     *
     *  A test that wants color asks for it locally, via default_style_guard or
     *  sink.with_style().
     **/
    const bool s_plain_pp_style = []() {
        xo::pp::PpStyle::default_style() = xo::pp::PpStyle::plain();
        return true;
    }();
}


/* end ppsink_utest_main.cpp */
