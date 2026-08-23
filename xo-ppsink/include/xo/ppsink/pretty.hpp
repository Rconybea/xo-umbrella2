/** @file pretty.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  pretty(sink, x): the dispatch verb for structured printing.
 *
 *  Separate from Prettifier.hpp so the two
 *  are easy to tell apart:
 *  1. specialize @c Prettifier<T> to teach a type how to print;
 *  2. include this header and call @c pretty() to actually print one.
 *
 *  Only the fallback operator<< fallback needs <ostream>, and only at its point of
 *  instantiation -- so this header itself does not include <ostream>.
 **/

#pragma once

#include "Prettifier.hpp"
#include <string_view>
#include <type_traits>

namespace xo::pp {
    /** Emit @p x to @p sink.
     *
     *  Dispatch, in order:
     *  1. @c Prettifier<T> when the type opts in (@ref has_prettifier) -- the
     *     native path: writes directly to the PpSink and can emit begin/split/end.
     *  2. string-like leaf (convertible to std::string_view) -- straight to
     *     PpSink::put, no ostream.
     *  3. fallback: operator<< via a streamed token -- available only when the
     *     translation unit included pretty_ostream.hpp, which is where
     *     operator<<(PpSinkInserter&, const T&) is declared.  Prefer a
     *     Prettifier<T> specialization so a type never lands here.
     *
     *  A type that reaches neither 1, 2 nor 3 is a compile error, not a
     *  placeholder rendering -- deliberately.  Whether 3 is available is a
     *  property of the TU, so a silent fallback would give pretty<T> different
     *  behaviour in different TUs, and the linker would pick one arbitrarily.
     **/
    /** true iff an operator<<(PpSinkInserter&, ..) is visible here -- i.e.
     *  whether this translation unit included pretty_ostream.hpp.
     *
     *  NB this is a property of the TU, not of @p T: the inserter it looks for
     *  is unconstrained, for a reason recorded at its definition.  So a @p T
     *  with no operator<<(std::ostream&, const T&) still satisfies this, and
     *  fails later inside pretty_ostream.hpp instead of at the static_assert
     *  below.  Worth knowing when reading a failure: "no match for operator<<"
     *  pointing INTO pretty_ostream.hpp means the type is unstreamable; the
     *  static_assert means the header is simply not included.
     **/
    template <typename T>
    concept pp_streamable = requires (PpSinkInserter & ins, const T & x) {
        ins << x;
    };

    template <typename T>
    void pretty(PpSink & sink, const T & x) {
        static_assert(!prettifier_broken<T>, "Prettifier<T> specialized but Prettifier<T>::print() not callable");

        if constexpr (has_prettifier<T>) {
            Prettifier<T>::print(sink, x);
        } else if constexpr (std::is_convertible_v<T, std::string_view>) {
            sink.put(std::string_view(x));
        } else if constexpr (pp_streamable<T>) {
            auto ins = sink.stream_open(1 /*min_z*/);
            ins << x;
            /* PpSinkInserter dtor commits the token */
        } else {
            static_assert(pp_streamable<T>,
                          "xo::pp::pretty: don't know how to render this type. "
                          "Either specialize Prettifier<T> (preferred: it composes, "
                          "and can emit break points), or #include "
                          "<xo/ppsink/pretty_ostream.hpp> in this translation unit "
                          "to render it as one opaque token via operator<<.");
        }
    }

    /** Use:
     *    PpSink & sink = ...;
     *
     *    sink.dwim(begin(2));
     **/
    struct begin {
        begin(std::uint32_t o) : offset_{o} {}

        std::uint32_t offset_;
    };

    /** Use:
     *    PpSink & sink = ...;
     *
     *    sink.dwim(split(1, 2));
     **/
    struct split {
        split(std::uint32_t s, std::int32_t o) : spaces_{s}, offset_{o} {}

        std::uint32_t spaces_;
        std::int32_t offset_;
    };

    /** Use:
     *    PpSink & sink = ...;
     *
     *    sink.newline(newline(2));
     **/
    struct newline {
        newline(std::int32_t o) : offset_{o} {}

        std::int32_t offset_;
    };

    namespace detail {
        struct _end {};
    }

    static inline constexpr detail::_end end;

    /** Use:
     *    PpSink & sink = ...;
     *
     *    sink.dwim("foo");       // default: same as pretty(sink,"foo");
     *    sink.dwim(begin(..));   // dwim: begin group
     *    sink.dwim(split(..));   // dwim: split to fit
     *    sink.dwim(newline(..)); // dwim: forced newline
     *    sink.dwim(end);         // dwim: end group
     **/
    template <typename T>
    void dwim(PpSink & sink, const T & x) {
        sink.pp(x);
    }

    /** sink.dwim(begin(...)) **/
    template <>
    void dwim(PpSink & sink, const begin & x);

    /** sink.dwim(split(...)) **/
    template <>
    void dwim(PpSink & sink, const split & x);

    /** sink.dwim(newline(...)) **/
    template <>
    void dwim(PpSink & sink, const newline & x);

    /** sink.dwim(end) **/
    template <>
    void dwim(PpSink & sink, const detail::_end & x);

    /** member-convenience spelling of pretty(*this, x); see PpSink::pp **/
    template <typename T>
    PpSink &
    PpSink::pp(const T & x) {
        pretty(*this, x);
        return *this;
    }

    /** member-convenience spelling of dwim(*this, x); see PpSink::dwim.
     *  NB: qualify the free function -- unqualified 'dwim' would bind to this
     *  member (member name hides the namespace-scope function).
     **/
    template <typename Dwim>
    void
    PpSink::dwim(const Dwim & x) {
        xo::pp::dwim(*this, x);
    }

    /** apply dwim() to each argument left-to-right; see PpSink::operator().
     *  (comma-operator fold sequences the calls left-to-right.)
     **/
    template <typename... Ts>
    void
    PpSink::operator()(const Ts &... args) {
        (this->dwim(args), ...);
    }

} /*namespace xo::pp*/

/* end pretty.hpp */
