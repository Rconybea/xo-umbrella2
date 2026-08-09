/* example ex3b/ex3b.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * xo-ppsink port of xo-indentlog/example/ex3, slice B: color.
 *
 * Same recursion + tags as ex3a, but scope entry/exit banners and tag names
 * are colored via ANSI escapes.  Color is arena-free (the escapes are
 * non-printing -- PpState::count_visible_chars skips them), so this runs
 * through the default FlatSink to std::clog.
 *
 * Run in a terminal to see color; piped/captured, the raw \033[...m escapes
 * are visible in the output.
 */

#include <xo/ppsink/color.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::PpStyle;
using xo::pp::color_spec_type;
using xo::pp::log_level;
using xo::pp::tag;
using xo::pp::xtag;

int
fib(int n) {
    scope log(XO_ENTER0_(info), tag("n", n));

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
    }

    log.end_scope(tag("n", n), " <-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 4;
    scope_config::function_entry_color = color_spec_type::xterm(69);   /* entry: blue-ish */
    scope_config::function_exit_color  = color_spec_type::xterm(70);   /* exit:  green-ish */
    PpStyle::default_style().color_enabled = true;
    PpStyle::default_style().tag_color = color_spec_type::xterm(166);  /* tag name: orange */

    int n = 3;

    scope log(XO_ENTER0_(info), tag("n", n));

    int fn = fib(n);

    log && log(tag("n", n));
    log && log("<-", xtag("fib(n)", fn));
}

/* end ex3b.cpp */
