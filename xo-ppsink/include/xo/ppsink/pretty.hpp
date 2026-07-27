/** @file pretty.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  pretty(sink, x): the dispatch verb for structured printing.
 *
 *  Kept separate from Prettifier.hpp (the customization *class*) so the two
 *  are easy to tell apart: specialize @c Prettifier<T> to teach a type how to
 *  print; include this header and call @c pretty() to actually print one.
 *
 *  Only the operator<< fallback needs <ostream>, and only at its point of
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
     *  3. fallback: operator<< via a streamed token.  This is the ONLY path
     *     that needs <ostream> (at the point of instantiation); prefer a
     *     Prettifier<T> specialization so a type never lands here.
     **/
    template <typename T>
    void pretty(PpSink & sink, const T & x) {
        if constexpr (has_prettifier<T>) {
            Prettifier<T>::print(sink, x);
        } else if constexpr (std::is_convertible_v<T, std::string_view>) {
            sink.put(std::string_view(x));
        } else {
            auto ins = sink.stream_open(1 /*min_z*/);
            ins << x;
            /* PpSinkInserter dtor commits the token */
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
    void
    PpSink::pp(const T & x) {
        pretty(*this, x);
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
