/** @file PpStyle.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  PpStyle: collects pretty-printer configuration settings.
 *
 *  Located here instead of in PpConfig to distinguish formatting from color
 *  Also necessary to use two levels because PpConfig depends on ArenaConfig,
 *  and we have leveling topology
 *    xo-ppsink <- xo-arena <- xo-indentlog2
 **/

#pragma once

#include "color_spec.hpp"
#include <cstdint>

namespace xo::pp {
    /** @brief presentation choices in force for a sink.
     *
     *  Held BY VALUE by PpSink (and by PpConfig), so there is no lifetime
     *  question and no shared mutable state between two sinks.
     *
     *  Process-wide defaults are @ref default_style; a PpSink or PpConfig
     *  copies them AT CONSTRUCTION, so changing the defaults afterwards does
     *  not reach a config already built.  Set them early (in main()) or set
     *  the style on the config.
     **/
    struct PpStyle {
        /** determine whether colored output is enabled **/
        bool color_enabled = true;

        /** color for the ":name" of a log tag, e.g. xtag("i", 5).
         *
         *  xterm 245 (grey) matches legacy xo::tag_config::tag_color
         *  (xo/indentlog/print/tag_config.hpp:34).
         **/
        color_spec_type tag_color = color_spec_type::xterm(245);

        /** color for the ":name" of a STRUCT FIELD, i.e. sink.pretty_struct().
         *
         *  Deliberately distinct from @ref tag_color, and yellow, matching
         *  legacy pretty_struct (xo/indentlog/print/pretty.hpp:407,425).
         *  Legacy hardcoded it, with `// tag_config::tag_color` left commented
         *  beside it, because it began as a diagnostic; RC kept it after the
         *  original motivation went away, finding the tag/field distinction
         *  worth having.  So: inherited AND chosen.
         **/
        color_spec_type struct_tag_color = color_spec_type::yellow();

        /** extra indent for a tag's or field's value when it breaks onto its
         *  own line (the offset passed to split()).
         *
         *  Named for its context rather than generically: this is the offset
         *  BELOW A TAG NAME, not a general-purpose one.  At 1, a broken value
         *  hangs one column past its ":name"; legacy instead added another full
         *  indent_width (2).  That difference is pinned in
         *  xo-object2/utest/printable_render.test.cpp -- see
         *  .xo-backlog/xo-printable2/issues/01.
         **/
        std::int32_t tag_value_offset = 1;

        /** a style with no color at all, layout defaults unchanged.
         *
         *  For anything pinning rendered TEXT -- a unit test, a log file, a pipe.
         **/
        static PpStyle plain() {
            PpStyle retval;

            retval.color_enabled = false;
            retval.tag_color = color_spec_type::none();
            retval.struct_tag_color = color_spec_type::none();

            return retval;
        }

        /** style with hardwired default colors **/
        static PpStyle colored() {
            PpStyle retval;

            retval.color_enabled = true;
            retval.tag_color = color_spec_type::xterm(245);
            retval.struct_tag_color = color_spec_type::yellow();

            return retval;
        }

        /** the process-wide defaults, from which every PpSink and PpConfig is
         *  initialised.  Mutable, so a program can restyle globally:
         *  @code
         *    PpStyle::default_style().tag_color = color_spec_type::xterm(166);
         *  @endcode
         **/
        static PpStyle & default_style() {
            static PpStyle s_instance;
            return s_instance;
        }
    };

    /** @brief RAII: replace PpStyle::default_style() for a region, restoring it
     *  on exit.
     *
     *  @code
     *    default_style_guard plain(PpStyle::plain());
     *  @endcode
     *
     *  Reaches what a sink-level with_style() cannot: the convenience entry
     *  points (tostr(), the operator<< bridge in tag_ostream.hpp) build their
     *  own FlatSink internally, so the only way to style them is through the
     *  defaults they copy from.  Where the sink IS in hand, prefer
     *  sink.with_style(); where a PpConfig is, prefer
     *  PpConfig::with_style() -- both are scoped by construction rather than by
     *  a destructor firing.
     **/
    class default_style_guard {
    public:
        explicit default_style_guard(const PpStyle & style)
            : orig_{PpStyle::default_style()}
        {
            PpStyle::default_style() = style;
        }
        ~default_style_guard() { PpStyle::default_style() = orig_; }

        default_style_guard(const default_style_guard &) = delete;
        default_style_guard & operator=(const default_style_guard &) = delete;

    private:
        PpStyle orig_;
    };
} /*namespace xo::pp*/

/* end PpStyle.hpp */
