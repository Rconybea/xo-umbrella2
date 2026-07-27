/** @file scope.cpp **/

#include <xo/ppsink/scope.hpp>
#include <xo/timeutil/timeutil_iostream.hpp>
#include <string>

namespace xo::pp {
    void
    scope::emit_indent(xo::pp::LogState & st) {
        std::uint32_t n = st.nesting_level() * scope_config::indent_width;
        if (n > scope_config::max_indent_width)
            n = scope_config::max_indent_width;   /* cap deep nesting */
        std::string pad(n, ' ');
        st.sink().put(pad);
    }

    void
    scope::emit_time(xo::pp::PpSink & sink, bool real_time) {
        if (!scope_config::time_enabled)
            return;

        /* "HH:MM:SS.mmm " = 13, "HH:MM:SS.uuuuuu " = 16 */
        std::uint32_t width = (scope_config::time_usec_flag ? 16 : 13);

        if (real_time) {
            xo::time::utc_nanos now = xo::time::timeutil::now();

            auto ins = sink.stream_open(width);
            if (scope_config::time_usec_flag)
                xo::time::print_hms_usec(xo::time::timeutil::utc_split_vs_midnight(now).second,
                                         ins.os());
            else
                xo::time::print_utc_hms_msec(now, ins.os());
            ins.os() << ' ';
            /* ins dtor commits the token */
        } else {
            /* mid-scope log() line: blank pad so it aligns under the banner */
            std::string pad(width, ' ');
            sink.put(pad);
        }
    }
} /*namespace xo::pp*/

/* end scope.cpp */
