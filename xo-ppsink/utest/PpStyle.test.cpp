/** @file PpStyle.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  PpStyle: presentation choices carried by a sink (xo/ppsink/PpStyle.hpp).
 *
 *  THIS FILE IS THE ONE PLACE THE DEFAULTS ARE ASSERTED.  Every utest main in
 *  the tree installs PpStyle::plain() so that expectations can pin readable
 *  text, which means the real defaults are exercised nowhere else -- they could
 *  be changed to none() and the entire suite would stay green.  That is exactly
 *  how ppsink's color gate came to be false unnoticed.
 *
 *  So these cases construct the styles they assert on, rather than reading the
 *  ambient default.
 **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::PpStyle;
    using xo::pp::color_encoding;
    using xo::pp::color_spec_type;
    using xo::pp::default_style_guard;
    using xo::pp::field;
    using xo::pp::tag;
    using std::stringstream;

    TEST_CASE("PpStyle-defaults", "[PpStyle]")
    {
        /* a freshly constructed PpStyle, NOT PpStyle::default_style() -- the
         * utest main has replaced that one
         */
        PpStyle dflt;

        /* grey, matching legacy xo::tag_config::tag_color */
        REQUIRE(dflt.tag_color.encoding() == color_encoding::xterm);
        REQUIRE(dflt.tag_color.code() == 245);

        /* yellow, matching legacy pretty_struct -- and DISTINCT from tag_color,
         * which is the point of having two
         */
        REQUIRE(dflt.struct_tag_color.encoding() == color_encoding::ansi);
        REQUIRE(dflt.struct_tag_color.code() == 33);

        REQUIRE(dflt.tag_value_offset == 1);
    }

    TEST_CASE("PpStyle-plain-is-colorless", "[PpStyle]")
    {
        PpStyle plain = PpStyle::plain();

        REQUIRE(plain.tag_color.encoding() == color_encoding::none);
        REQUIRE(plain.struct_tag_color.encoding() == color_encoding::none);

        /* layout is NOT a color decision: plain() must not disturb it */
        REQUIRE(plain.tag_value_offset == PpStyle().tag_value_offset);
    }

    TEST_CASE("PpStyle-tag-vs-struct-field", "[PpStyle]")
    {
        /* the distinction the two colors exist for: the same ":name value"
         * shape renders in different colors depending on whether it is a log
         * tag or a struct field.
         */
        PpStyle style;
        style.tag_color = color_spec_type::ansi(31);          /* red */
        style.struct_tag_color = color_spec_type::ansi(32);   /* green */

        int x = 1;

        stringstream tag_ss;
        {
            FlatSink sink(style, tag_ss);
            sink.pp(tag("k", x));
        }

        stringstream field_ss;
        {
            FlatSink sink(style, field_ss);
            sink.pretty_struct("P", field("k", x));
        }

        REQUIRE(tag_ss.str() == "\033[31m:k\033[0m 1");
        REQUIRE(field_ss.str() == "<P \033[32m:k\033[0m 1>");
    }

    TEST_CASE("PpStyle-per-sink-not-global", "[PpStyle]")
    {
        /* two sinks alive at once, styled differently.  This is what the old
         * process-wide tag_config could not do, and the reason PpStyle is held
         * by value on the sink.
         */
        PpStyle red;
        red.tag_color = color_spec_type::ansi(31);

        stringstream styled_ss, plain_ss;
        FlatSink styled(red, styled_ss);
        FlatSink plain(PpStyle::plain(), plain_ss);

        int x = 1;
        styled.pp(tag("k", x));
        plain.pp(tag("k", x));

        REQUIRE(styled_ss.str() == "\033[31m:k\033[0m 1");
        REQUIRE(plain_ss.str() == ":k 1");
    }

    TEST_CASE("PpStyle-sink-copies-defaults-at-construction", "[PpStyle]")
    {
        /* a sink takes the defaults WHEN IT IS BUILT.  Someone who sets the
         * defaults late, after building a sink, gets nothing -- worth pinning,
         * because the failure is silent.
         */
        PpStyle red;
        red.tag_color = color_spec_type::ansi(31);

        stringstream before_ss, after_ss;

        FlatSink before(before_ss);      /* built under plain (the utest main) */

        default_style_guard g(red);

        FlatSink after(after_ss);        /* built under red */

        int x = 1;
        before.pp(tag("k", x));
        after.pp(tag("k", x));

        REQUIRE(before_ss.str() == ":k 1");
        REQUIRE(after_ss.str() == "\033[31m:k\033[0m 1");
    }

    TEST_CASE("PpStyle-default-style-guard-restores", "[PpStyle]")
    {
        PpStyle orig = PpStyle::default_style();

        {
            PpStyle red;
            red.tag_color = color_spec_type::ansi(31);

            default_style_guard g(red);

            REQUIRE(PpStyle::default_style().tag_color.code() == 31);
        }

        REQUIRE(PpStyle::default_style().tag_color.encoding() == orig.tag_color.encoding());
        REQUIRE(PpStyle::default_style().tag_color.code() == orig.tag_color.code());
    }

    TEST_CASE("PpStyle-tag-value-offset", "[PpStyle]")
    {
        /* tag_value_offset is layout, not color: it decides how far a broken
         * value hangs below its ":name".  Only visible through a sink that
         * breaks, so the rendered check lives in xo-indentlog2; here we pin
         * that the field is reached from the sink at all.
         */
        PpStyle style;
        style.tag_value_offset = 4;

        stringstream ss;
        FlatSink sink(style, ss);

        REQUIRE(sink.style().tag_value_offset == 4);
        REQUIRE(FlatSink(ss).style().tag_value_offset == 1);   /* untouched sink */
    }
} /*namespace ut*/

/* end PpStyle.test.cpp */
