/** @file color.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/color.hpp>
#include <cstdio>

namespace xo::pp {
    void
    color_spec_type::fg_on(PpSink & sink) const
    {
        if (encoding_ == color_encoding::none)
            return;

        /* build the whole escape in one buffer, then put() as a single token
         * (count_visible_chars only skips an escape within one token).
         * worst case "\033[38;2;255;255;255m" is 19 chars.
         */
        char buf[32];
        int len = 0;

        switch (encoding_) {
        case color_encoding::none:
            return;
        case color_encoding::ansi:
            len = std::snprintf(buf, sizeof(buf), "\033[%um",
                                static_cast<unsigned>(code_));
            break;
        case color_encoding::xterm:
            len = std::snprintf(buf, sizeof(buf), "\033[38;5;%um",
                                static_cast<unsigned>(code_));
            break;
        case color_encoding::rgb:
            len = std::snprintf(buf, sizeof(buf), "\033[38;2;%u;%u;%um",
                                static_cast<unsigned>(0xff & (code_ >> 16)),
                                static_cast<unsigned>(0xff & (code_ >> 8)),
                                static_cast<unsigned>(0xff & (code_ >> 0)));
            break;
        }

        if (len > 0)
            sink.put(std::string_view(buf, static_cast<std::size_t>(len)));
    }

    void
    color_spec_type::fg_off(PpSink & sink) const
    {
        if (encoding_ == color_encoding::none)
            return;

        sink.put(std::string_view("\033[0m", 4));
    }
} /*namespace xo::pp*/

/* end color.cpp */
