/** @file quoted_char.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  quoted_char(ch): print one character readably, escaping it if it has no
 *  printable form.  The ppsink-native replacement for the legacy
 *  xo-indentlog xo::quoted_char.
 *
 *  Renders through PpSink::put_with_escape(), so it follows the same rules as
 *  every other escaped string in ppsink (see escape.hpp).  Note that differs
 *  from the legacy version in one respect: ESC prints as @c \x1b rather than
 *  octal @c \033, and characters the legacy version passed through raw (tab,
 *  NUL, ...) are now escaped.  One escape vocabulary throughout is worth the
 *  small divergence.
 *
 *  Despite the name, no surrounding quotes are added -- "quoted" here means
 *  escaped-for-display, matching the legacy behavior.
 *
 *  This header is ostream-free; see quoted_char_ostream.hpp to stream one to
 *  an ostream.
 **/

#pragma once

#include "Prettifier.hpp"
#include "escape.hpp"
#include <string_view>

namespace xo::pp {
    /** @class quoted_char
     *  @brief One character, to be printed readably.
     *
     *  Use:
     *  @code
     *    sink.pp(quoted_char(ch));                  // into a PpSink
     *    cerr << quoted_char(ch);                   // needs quoted_char_ostream.hpp
     *  @endcode
     **/
    class quoted_char {
    public:
        explicit constexpr quoted_char(char ch) noexcept : ch_{ch} {}

        constexpr char ch() const noexcept { return ch_; }

    private:
        /** the character to print **/
        char ch_;
    };

    /** render a quoted_char into @p sink, escaped but unquoted **/
    template <>
    struct Prettifier<quoted_char> {
        static void print(PpSink & sink, const quoted_char & x) {
            /* a local, so the string_view can't outlive what it points at */
            char ch = x.ch();

            sink.put_with_escape(std::string_view(&ch, 1), false /*quote_flag*/);
        }
    };
} /*namespace xo::pp*/

/* end quoted_char.hpp */
