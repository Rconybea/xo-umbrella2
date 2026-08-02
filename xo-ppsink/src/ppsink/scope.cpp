/** @file scope.cpp **/

#include <xo/ppsink/pp_time.hpp>
#include <xo/ppsink/scope.hpp>
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

            /* time-of-day since midnight, in local or UTC coords */
            xo::time::nanos since_midnight =
                (scope_config::time_local_flag
                 ? xo::time::timeutil::local_split_vs_midnight(now).second
                 : xo::time::timeutil::utc_split_vs_midnight(now).second);

            /* PpSink-native (single put token, no ostream fallback) */
            if (scope_config::time_usec_flag)
                put_hms_usec(sink, since_midnight);
            else
                put_hms_msec(sink, since_midnight);
            sink.put(" ");
        } else {
            /* mid-scope log() line: blank pad so it aligns under the banner */
            std::string pad(width, ' ');
            sink.put(pad);
        }
    }

    void
    scope::emit_nesting_level(xo::pp::PpSink & sink, std::uint32_t level) {
        if (!scope_config::nesting_level_enabled)
            return;

        /* "(" <colored N> ") " -- parens/space plain, depth colored (legacy style) */
        sink.put("(");
        {
            color_guard g(sink, scope_config::nesting_level_color);
            sink.pp(static_cast<int>(level));
        }
        sink.put(") ");
    }

    void
    scope::emit_location(xo::pp::PpSink & sink,
                         std::string_view file, std::uint32_t line) {
        if (!scope_config::location_enabled || file.empty())
            return;

        /* basename: portion after the last '/' */
        std::size_t slash = file.find_last_of('/');
        std::string_view base = (slash == std::string_view::npos)
            ? file : file.substr(slash + 1);

        /* right-align at location_tab when the sink knows its column;
         * otherwise (FlatSink) fall back to a single inline space.
         */
        std::optional<std::size_t> col = sink.lpos();
        if (col.has_value()) {
            std::size_t tab = scope_config::location_tab;
            std::size_t cur = *col;
            std::size_t pad = (cur < tab) ? (tab - cur) : 1;   /* >= 1 space */
            sink.put(std::string(pad, ' '));
        } else {
            sink.put(" ");
        }

        color_guard g(sink, scope_config::code_location_color);
        sink("[", base, ":", static_cast<int>(line), "]");
        //sink.put("[");
        //sink.put(base);
        //sink.put(":");
        //sink.pp(static_cast<int>(line));
        //sink.put("]");
    }
} /*namespace xo::pp*/

/* end scope.cpp */
