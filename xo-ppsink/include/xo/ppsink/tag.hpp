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
 *  POC subset: autoescape of the value (legacy `unq`) and name-coloring
 *  (legacy `with_color` + tag_config) are deferred to later slices.
 *
 *  NB: namespace xo::pp (not xo) to avoid colliding with the legacy
 *  xo::tag when both are visible in one translation unit.
 **/

#pragma once

#include "pretty.hpp"
#include <utility>
#include <type_traits>

namespace xo::pp {
    /** @brief value-escaping policy for a tag (POC: recorded, not yet applied) **/
    enum class tagstyle {
        /** escape embedded whitespace/special chars; quote if needed **/
        autoescape,
        /** print the value literally **/
        raw,
    };

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

            sink.begin();
            sink.put(":");
            sink.put(t.name());
            sink.split(1);   /* 1 space if it fits; newline + indent if not */
            pretty(sink, t.value());
            sink.end();
        }
    };
} /*namespace xo::pp*/

/* end tag.hpp */
