/** @file pretty_struct.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  sink.pretty_struct("Name", fields...): render a struct-shaped value as
 *
 *      <Name :f1 v1 :f2 v2>
 *
 *  breaking, when it does not fit the margin, to
 *
 *      <Name
 *        :f1 v1
 *        :f2 v2>
 *
 *  The ppsink replacement for legacy xo::print::ppstate::pretty_struct
 *  (xo/indentlog/print/pretty.hpp).  Two differences from the legacy verb:
 *
 *  1. no trial-fit pass.  Legacy print_pretty() was called twice -- once with
 *     ppii.upto() true to see whether the group fit, then again to emit.  Here
 *     the caller emits ONE begin/split/end token stream and PpState decides
 *     downstream whether each group breaks.  All upto() branching disappears.
 *
 *  2. returns void, not bool.
 *
 *  Own header (not folded into pretty.hpp / Prettifier.hpp) because this is a
 *  layout *vocabulary* built on top of the dispatch verb, not part of it: a
 *  Prettifier author includes this only when the type is struct-shaped.
 *  Follows the pattern of PpSink::pp / dwim / operator(), which are likewise
 *  declared in PpSink.hpp and defined in the header that supplies them.
 **/

#pragma once

#include "tag.hpp"    /* Prettifier, PpSink, pretty(), color_guard, tag_config */
#include <concepts>
#include <string_view>

namespace xo::pp {

    // ------------------------------------------------------------------ field

    /** @brief one named field of a struct, holding its value BY REFERENCE.
     *
     *  The ppsink counterpart to legacy xo::refrtag.  Two differences from
     *  xo::pp::tag:
     *
     *  1. by reference, not by value.  tag() decays and copies its value (see
     *     tag.hpp), which for a std::string member means a heap allocation on
     *     every print, and for a move-only member is a compile error.  A
     *     struct printer names *members*, so a reference is right.
     *
     *     LIFETIME: @p value must outlive the pretty_struct() call.  Binding a
     *     prvalue directly in the call expression is safe -- the temporary
     *     lives to the end of the full-expression -- but is easy to break when
     *     refactoring, so prefer passing a member or a named local.  In
     *     particular, an accessor that returns BY VALUE (e.g. `metatype()`)
     *     wants a named local.
     *
     *  2. carries a @p present flag, so a field can be omitted entirely
     *     (legacy refrtag's third argument).
     *
     *  Anything renderable can be a field: pretty_struct() prints each field
     *  through pretty(), and only *consults* present() when the field type has
     *  one.  So tag("k", v) is a perfectly good field when a copy is wanted.
     **/
    template <typename Value>
    class field_impl {
    public:
        field_impl(std::string_view name, const Value & value, bool present)
            : name_{name}, value_{&value}, present_{present} {}

        std::string_view name() const { return name_; }
        const Value & value() const { return *value_; }
        bool present() const { return present_; }

    private:
        std::string_view name_;
        const Value * value_;
        bool present_ = true;
    };

    /** ":name value" naming a struct member; @p value captured BY REFERENCE
     *  (see field_impl for the lifetime rule).  Omitted from the output
     *  entirely when @p present is false.
     **/
    template <typename Value>
    field_impl<Value>
    field(std::string_view name, const Value & value, bool present = true) {
        return field_impl<Value>(name, value, present);
    }

    /** render ":name value" -- same shape as Prettifier<tag_impl>, without the
     *  copy, and without tag's optional leading space (pretty_struct emits the
     *  separator itself).
     **/
    template <typename Value>
    struct Prettifier<field_impl<Value>> {
        static void print(PpSink & sink, const field_impl<Value> & f) {
            sink.begin(0);
            {
                /* color just the ":name" (value keeps its own color/structure) */
                color_guard g(sink, tag_config::tag_color);
                sink.put(":");
                sink.put(f.name());
            }
            sink.split(1, tag_config::value_offset);
            sink.pp(f.value());
            sink.end();
        }
    };

    // ---------------------------------------------------------- pretty_struct

    namespace detail {
        /** true iff F reports its own presence (legacy xo::has_present).
         *
         *  Deliberately NOT named has_present: xo::pp nests inside xo, so an
         *  unqualified use would also find legacy ::xo::has_present when
         *  <xo/indentlog/print/tag.hpp> is in the same translation unit.
         **/
        template <typename F>
        concept has_present_flag = requires (const F & f) {
            { f.present() } -> std::convertible_to<bool>;
        };

        template <typename F>
        inline void pretty_struct_field(PpSink & sink, const F & f) {
            if constexpr (has_present_flag<F>) {
                if (!f.present())
                    return;      /* absent: drop the field AND its separator */
            }
            sink.split(1);
            sink.pp(f);
        }
    } /*namespace detail*/

    /** see PpSink::pretty_struct.
     *
     *  Emits, in order:
     *      put("<") put(name) begin()
     *        [ split(1) pp(field) ]*        -- absent fields skipped entirely
     *      put(">") end()
     *
     *  Four shape rules, each easy to get subtly wrong:
     *
     *  - put(name) precedes begin(), so the header stays on the opening line
     *    when the group breaks.
     *
     *  - begin() takes no offset: the indent comes from the sink's own
     *    PpConfig::indent_width, and *accumulates*, so a struct nested inside
     *    a struct indents one further level.
     *
     *  - split(1), not split(1, indent).  begin() has already moved the
     *    running indent and PpState adds a split's offset ON TOP of it, so
     *    passing the indent to both double-counts (broken fields land at 2x
     *    the intended column).
     *
     *  - split(1) before EVERY field including the first, matching legacy,
     *    which emits " " ahead of each member.  Without it the first field
     *    abuts the name: <Name:f1 v1 ...>.
     **/
    template <typename... Fields>
    void
    PpSink::pretty_struct(std::string_view name, const Fields &... fields) {
        this->put("<");
        this->put(name);
        this->begin();
        (detail::pretty_struct_field(*this, fields), ...);
        this->put(">");
        this->end();
    }

} /*namespace xo::pp*/

/* end pretty_struct.hpp */
