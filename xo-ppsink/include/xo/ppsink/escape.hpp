/** @file escape.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Escape rules for PpSink::put_with_escape(), in one place.
 *
 *  Both PpSink implementations need these rules -- FlatSink expands straight to
 *  its ostream, PrettySink/PpState expands into a PpStringToken -- but the token
 *  mechanics differ, so only the *rules* are shared here.  Two hand-written
 *  escape switches would drift.
 *
 *  Escaping only; no surrounding quotes.  Whether to quote (never / only when
 *  ambiguous / always) is a separate policy that belongs above this layer.
 *  Note in particular that space is NOT escaped: an embedded space is a reason
 *  to *quote*, which is that upper layer's concern.
 *
 *  This header is ostream-free, like the rest of the ppsink core.
 **/

#pragma once

#include <string_view>
#include <cstdint>

namespace xo::pp {
    /** @class Escape
     *  @brief Escape rules for @ref PpSink::put_with_escape.
     *
     *  A class used as a namespace: no instances, all members static.
     *  Grouped this way so the rules, the size scan, the copy, and the
     *  expansion bound all sit together and stay consistent.
     *
     *  Use:
     *  @code
     *    auto [esc_z, esc_flag] = Escape::str_size(s);
     *    char * buf = ...;                 // esc_z bytes
     *    char * end = Escape::str_copy(s, buf);   // end - buf == esc_z
     *  @endcode
     **/
    class Escape {
    public:
        /** @defgroup escape-constants constants **/
        ///@{

        /** longest expansion of a single input character, in bytes.
         *  Attained by the @c \xNN form (see @ref needs_hex).
         *
         *  A caller expanding incrementally into a fixed buffer must keep this
         *  many bytes in reserve before handling the next character.
         **/
        static constexpr std::uint32_t c_max_char_expand = 4;

        /** surrounding quote character, when a caller asks to quote.
         *
         *  Lives here, next to @ref pair_char, because the two must agree:
         *  whatever delimits the string has to be escaped inside it.
         **/
        static constexpr char c_quote = '"';

        /** bytes added by surrounding quotes: one at each end **/
        static constexpr std::uint32_t c_quote_expand = 2;

        ///@}
        /** @defgroup escape-types types **/
        ///@{

        /** @brief result of @ref str_size.
         *
         *  Supports structured binding, so a caller that wants both can write
         *  @code auto [esc_z, esc_flag] = Escape::str_size(s); @endcode
         **/
        struct size_result {
            /** number of bytes @ref str_copy will write **/
            std::uint32_t size = 0;
            /** true iff printing this string bare would be ambiguous, so the
             *  caller has to surround it with @ref c_quote.  This is the unq()
             *  predicate.  True when any of:
             *
             *  - a character was escaped, since a backslash or a quote in the
             *    output needs delimiters to be read back;
             *  - the string contains a space, which bare would read as a
             *    separator;
             *  - the string is empty, which bare would be nothing at all.
             *
             *  Note the name is looser than it reads: the last two cases set
             *  this without anything actually being escaped, so it is a
             *  "needs quoting" flag rather than a "was modified" flag.  It is
             *  therefore NOT a test for @c str_copy(s) == s.
             **/
            bool escape_flag = false;
        };

        ///@}
        /** @defgroup escape-rules per-character rules **/
        ///@{

        /** @brief second character of @p uc's two-character escape, or 0 if none.
         *
         *  Covers the four escapes with a short form:
         *  @code \ -> \\   " -> \"   newline -> \n   cr -> \r @endcode
         *
         *  Consulted before @ref needs_hex, since newline and cr are also
         *  control characters and their short form is preferred.
         **/
        static constexpr char
        pair_char(unsigned char uc) noexcept {
            switch (uc) {
            case '\\': return '\\';
            case '"':  return '"';
            case '\n': return 'n';
            case '\r': return 'r';
            default:   return '\0';
            }
        }

        /** @brief true iff @p uc must be escaped as @c \xNN (no short form).
         *
         *  Control characters have no printable representation, and one of them
         *  -- ESC (0x1b) -- actively corrupts layout: PpState::count_visible_chars
         *  treats ESC as the start of a (zero-width) color escape, so a raw ESC
         *  in a string token makes its visible length undercount and breaks the
         *  pretty-printer's line-fitting.  Escaping them makes that unreachable.
         *
         *  Bytes >= 0x80 are deliberately left alone so UTF-8 passes through
         *  intact; note the unsigned char parameter, since plain char is signed
         *  on the platforms xo builds for.
         **/
        static constexpr bool
        needs_hex(unsigned char uc) noexcept {
            return (uc < 0x20) || (uc == 0x7f);
        }

        ///@}
        /** @defgroup escape-methods scan and copy **/
        ///@{

        /** @brief scan @p s: bytes @ref str_copy will write, and whether the
         *  result needs surrounding quotes to be unambiguous.
         *
         *  Single pass, no allocation.  Callers need the size before allocating,
         *  since PpState places a PpStringToken of known size and then fills it.
         *  @ref size_result::escape_flag is free to compute in the same pass,
         *  and is what unq() branches on.
         **/
        static constexpr size_result
        str_size(std::string_view s) noexcept {
            size_result retval;

            /* bare, an empty string would print as nothing at all */
            retval.escape_flag = s.empty();

            for (char ch : s) {
                auto uc = static_cast<unsigned char>(ch);

                if (pair_char(uc)) {
                    retval.size += 2;
                    retval.escape_flag = true;
                } else if (needs_hex(uc)) {
                    retval.size += c_max_char_expand;   /* \xNN */
                    retval.escape_flag = true;
                } else {
                    retval.size += 1;

                    /* not escaped, but bare it would read as a separator */
                    if (uc == ' ')
                        retval.escape_flag = true;
                }
            }

            return retval;
        }

        /** @brief write @p s to @p dest with escapes expanded; return one-past-end.
         *
         *  Requires space for @c str_size(s).size bytes at @p dest.  Writes
         *  exactly that many -- @ref str_size and this method branch on the same
         *  predicates in the same order, and the tests pin the equality.
         **/
        static constexpr char *
        str_copy(std::string_view s, char * dest) noexcept {
            for (char ch : s) {
                auto uc = static_cast<unsigned char>(ch);

                if (char pair = pair_char(uc)) {
                    *dest++ = '\\';
                    *dest++ = pair;
                } else if (needs_hex(uc)) {
                    *dest++ = '\\';
                    *dest++ = 'x';
                    *dest++ = c_hex_digit[(uc >> 4) & 0x0f];
                    *dest++ = c_hex_digit[uc & 0x0f];
                } else {
                    *dest++ = ch;
                }
            }

            return dest;
        }

        ///@}

    private:
        /** lowercase nibble -> hex digit, for the @c \xNN form **/
        static constexpr char c_hex_digit[] = "0123456789abcdef";
    };
} /*namespace xo::pp*/

/* end escape.hpp */
