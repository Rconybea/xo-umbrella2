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
    } /*namespace detail*/

    /** @brief RAII scope emitting  <Name :f1 v1 :f2 v2>  one field at a time.
     *
     *  The single place the struct token stream is produced: pretty_struct()
     *  is a thin variadic wrapper over this, so the four shape rules below are
     *  stated once.  Reach for struct_scope directly only when the field count
     *  is a runtime value -- a loop cannot contribute to a variadic pack.
     *
     *  Emits, in order:
     *      put("<") put(name) begin()
     *        [ separator  pp(field) ]*      -- absent fields skipped entirely
     *      put(">") end()                   -- from the destructor
     *
     *  Four shape rules, each easy to get subtly wrong (span_pp.hpp and commit
     *  e07b98b1 each got one wrong):
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
     *  - a separator before EVERY field including the first, matching legacy,
     *    which emits " " ahead of each member.  Without it the first field
     *    abuts the name: <Name:f1 v1 ...>.
     *
     *  The closing ">" belongs INSIDE the group (before end()), which the
     *  destructor makes impossible to forget.
     **/
    class struct_scope {
    public:
        /** @p force_break: separate fields with newline() rather than split(),
         *  so the struct always renders multi-line even where it would fit.
         *  A deliberate layout policy; see PpSink::struct_open.
         **/
        struct_scope(PpSink & sink, std::string_view name, bool force_break)
            : sink_{sink}, force_break_{force_break}
        {
            sink_.put("<");
            sink_.put(name);
            sink_.begin();
        }

        ~struct_scope() {
            sink_.put(">");
            sink_.end();
        }

        /* holds a PpSink & and owns an emission bracket: neither copyable nor
         * movable.  struct_open() returns one by value via guaranteed elision.
         */
        struct_scope(const struct_scope &) = delete;
        struct_scope & operator=(const struct_scope &) = delete;

        /** add ":name value"; omitted entirely when @p present is false **/
        template <typename Value>
        struct_scope & field(std::string_view name,
                             const Value & value,
                             bool present = true)
        {
            if (present) {
                this->separator();
                /* qualified: `field` is also this member's name */
                sink_.pp(xo::pp::field(name, value));
            }
            return *this;
        }

        /** add an already-built field-like value (anything renderable); its
         *  present() is consulted when it has one, as with pretty_struct().
         **/
        template <typename F>
        struct_scope & item(const F & f) {
            if constexpr (detail::has_present_flag<F>) {
                if (!f.present())
                    return *this;   /* absent: drop the field AND its separator */
            }
            this->separator();
            sink_.pp(f);
            return *this;
        }

    private:
        void separator() {
            if (force_break_)
                sink_.newline(0);
            else
                sink_.split(1, 0);
        }

    private:
        PpSink & sink_;
        /** true: always break between fields, regardless of fit **/
        bool force_break_ = false;
    }; /*struct_scope*/

    /** see PpSink::struct_open **/
    inline struct_scope
    PpSink::struct_open(std::string_view name, bool force_break) {
        return struct_scope(*this, name, force_break);
    }

    /** see PpSink::pretty_struct.  A compile-time field list is just a
     *  struct_scope that never force-breaks.
     **/
    template <typename... Fields>
    void
    PpSink::pretty_struct(std::string_view name, const Fields &... fields) {
        struct_scope st(*this, name, false /*force_break*/);

        (st.item(fields), ...);
    }

} /*namespace xo::pp*/

/* end pretty_struct.hpp */
