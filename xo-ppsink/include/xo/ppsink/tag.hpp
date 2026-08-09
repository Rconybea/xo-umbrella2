/** @file tag.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Key/value logging helper for the ppsink scope logger (POC subset ported
 *  from the legacy xo-indentlog tag).  A tag prints as
 *  @code :name value @endcode
 *
 *  Unlike the legacy tag (which customized printing via an ostream inserter),
 *  this one customizes via Prettifier<tag_impl> and writes straight into the
 *  PpSink.  That keeps tag.hpp free of <ostream>, lets the value render
 *  through pretty() (so a pretty-printable value keeps its own structure),
 *  and leaves room to introduce a split() point between :name and value.
 *
 *  POC subset: autoescape of the value (legacy `unq`) is deferred to a later
 *  slice.  Name-coloring is done, via PpStyle::tag_color (PpStyle.hpp).
 *
 *  NB: namespace xo::pp (not xo) to avoid colliding with the legacy
 *  xo::tag when both are visible in one translation unit.
 **/

#pragma once

#include "color.hpp"
#include "pretty.hpp"
#include "stringify.hpp" /* STRINGIFY -- generic macro, kept for tag/logging callers */
#include <type_traits>
#include <utility>

namespace xo::pp {
    /** @brief value-escaping policy for a tag (POC: recorded, not yet applied) **/
    enum class tagstyle {
        /** escape embedded whitespace/special chars; quote if needed **/
        autoescape,
        /** print the value literally **/
        raw,
    };

    /* NB: tag rendering configuration used to live here, as a process-wide
     * `tag_config`.  It is now PpStyle (PpStyle.hpp), reached per-sink through
     * PpSink::style(), so that a caller can restyle ONE render instead of the
     * whole process:
     *
     *     tag_config::tag_color     -> sink.style().tag_color
     *                                  PpStyle::default_style().tag_color
     *     tag_config::value_offset  -> sink.style().tag_value_offset
     *
     * Deliberately not left behind as an alias: two names for one setting is
     * how half a program ends up configuring the other half's copy.
     */

    /** @brief key/value pair for logging, printed as ":name value".
     *
     *  @tparam PrefixSpace  if true, emit one leading space before ":name"
     *  @tparam TagStyle     value-escaping policy (see @ref tagstyle)
     *  @tparam Name         key type (typically char const *)
     *  @tparam Value        value type
     **/
    template <bool PrefixSpace, tagstyle TagStyle, typename Name, typename Value>
    struct tag_impl {
        tag_impl(Name name, Value value)
            : name_{std::move(name)}, value_{std::move(value)} {}

        constexpr bool prefix_space() const { return PrefixSpace; }
        const Name & name() const { return name_; }
        const Value & value() const { return value_; }

    private:
        Name name_;
        Value value_;
    };

    /** ":name value" -- no leading space **/
    template <typename Name, typename Value>
    tag_impl<false, tagstyle::autoescape, std::decay_t<Name>, std::decay_t<Value>>
    tag(Name && name, Value && value) {
        return tag_impl<false, tagstyle::autoescape,
                        std::decay_t<Name>, std::decay_t<Value>>(
            std::forward<Name>(name), std::forward<Value>(value));
    }

    /** " :name value" -- with one leading space (for mid-line placement) **/
    template <typename Name, typename Value>
    tag_impl<true, tagstyle::autoescape, std::decay_t<Name>, std::decay_t<Value>>
    xtag(Name && name, Value && value) {
        return tag_impl<true, tagstyle::autoescape,
                        std::decay_t<Name>, std::decay_t<Value>>(
            std::forward<Name>(name), std::forward<Value>(value));
    }

    /** render a tag into the sink: ":name " then the value via pretty.
     *
     *  Writing the value through pretty() (rather than an ostream inserter)
     *  means a pretty-printable value keeps its own structure, and an int
     *  value renders ostream-free via Prettifier<int>.
     **/
    template <bool PrefixSpace, tagstyle TagStyle, typename Name, typename Value>
    struct Prettifier<tag_impl<PrefixSpace, TagStyle, Name, Value>> {
        static void print(PpSink & sink,
                          const tag_impl<PrefixSpace, TagStyle, Name, Value> & t) {
            if constexpr (PrefixSpace)
                sink.put(" ");

            sink.begin(0);
            {
                /* color just the ":name" (value keeps its own color/structure) */
                color_guard g(sink, sink.style().tag_color);
                sink.put(":");
                /* pp(), not put(): a name may be any renderable (e.g. concat,
                 * concat.hpp).  For a string-like name this is the same
                 * string-like leaf that put() would have taken.
                 */
                sink.pp(t.name());
            }
            /* 1 space if it fits; newline + tag_value_offset indent if not */
            sink.split(1, sink.style().tag_value_offset);
            sink.pp(t.value());
            sink.end();
        }
    };
} /*namespace xo::pp*/

/** tag a value with the text of the expression that produced it:
 *  @code XTAG_(width) @endcode prints as @code :width 7 @endcode
 *
 *  Mirrors legacy XTAG(x) = xo::xtag(STRINGIFY(x), x), and lives here for the
 *  same reason legacy put XTAG in print/tag.hpp -- it is part of the tag
 *  vocabulary, alongside the STRINGIFY included above.
 *
 *  Trailing underscore per the ppsink macro convention (see scope_macros.hpp),
 *  so a translation unit can pull in both this and legacy indentlog's XTAG.
 **/
#define XTAG_(x) xo::pp::xtag(STRINGIFY(x), x)

/* end tag.hpp */
