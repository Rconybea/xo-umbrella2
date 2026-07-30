/** @file quoted.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  quot(s) / unq(s): print a string with escapes, choosing whether to surround
 *  it with quotes.  The ppsink-native replacement for the legacy
 *  xo-indentlog xo::print::quot / xo::print::unq.
 *
 *  @code
 *    quot("a b")  ->  "a b"     // always quoted
 *    quot("ab")   ->  "ab"      //   ...even when unambiguous
 *    unq("a b")   ->  "a b"     // quoted: the space would read as a separator
 *    unq("ab")    ->  ab        // bare: unambiguous
 *  @endcode
 *
 *  Both apply the escape rules in escape.hpp; they differ only in the quoting
 *  policy.  unq() branches on Escape::size_result::escape_flag, so the
 *  "is this ambiguous bare" rule lives in one place.
 *
 *  Takes std::string_view (so const char * and std::string convert
 *  implicitly), rather than being templated over arbitrary T the way the legacy
 *  version was.  The legacy version rendered any value to a std::string via
 *  tostr() before inspecting it; in ppsink a non-string value goes through its
 *  own Prettifier instead, and every quot/unq call site in xo passes something
 *  string-like.
 *
 *  This header is ostream-free; see quoted_ostream.hpp to stream one directly
 *  to an ostream.  (Using one as a tag value -- @c xtag("k", unq(s)) -- needs
 *  only this header, since the tag machinery renders it through the sink.)
 **/

#pragma once

#include "escape.hpp"
#include "Prettifier.hpp"
#include <string_view>

namespace xo::pp {
    /** @brief when should a string be surrounded by Escape::c_quote **/
    enum class quotestyle {
        /** always quote, whether or not it would be ambiguous bare **/
        always,
        /** quote only when bare would be ambiguous (see escape_flag) **/
        if_needed,
    };

    /** @brief a string to print escaped, with quoting policy @p QuoteStyle **/
    template <quotestyle QuoteStyle>
    class quot_impl {
    public:
        explicit constexpr quot_impl(std::string_view s) noexcept : s_{s} {}

        constexpr std::string_view value() const noexcept { return s_; }

    private:
        /** the string to print **/
        std::string_view s_;
    };

    /** print @p s escaped and always quoted **/
    inline constexpr quot_impl<quotestyle::always>
    quot(std::string_view s) noexcept {
        return quot_impl<quotestyle::always>(s);
    }

    /** print @p s escaped, quoted only if bare would be ambiguous **/
    inline constexpr quot_impl<quotestyle::if_needed>
    unq(std::string_view s) noexcept {
        return quot_impl<quotestyle::if_needed>(s);
    }

    /** render a quot_impl into @p sink, applying its quoting policy **/
    template <quotestyle QuoteStyle>
    struct Prettifier<quot_impl<QuoteStyle>> {
        static void print(PpSink & sink, const quot_impl<QuoteStyle> & x) {
            bool quote_flag = true;

            if constexpr (QuoteStyle == quotestyle::if_needed)
                quote_flag = Escape::str_size(x.value()).escape_flag;

            sink.put_with_escape(x.value(), quote_flag);
        }
    };
} /*namespace xo::pp*/

/* end quoted.hpp */
