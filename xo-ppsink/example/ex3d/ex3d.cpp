/* example ex3d/ex3d.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * xo-ppsink port of xo-indentlog/example/ex3, slice F: max indent width.
 *
 * scope_config::max_indent_width caps total indentation, so deeply-nested
 * scopes stop marching rightward.  Here indent_width=2, max_indent_width=8,
 * so from nesting depth 4 on, the banner indent stays at 8 columns.
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::log_level;
using xo::pp::tag;

void
descend(int n) {
    scope log(XO_ENTER0_(info), tag("n", n));

    if (n > 0)
        descend(n - 1);
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;
    scope_config::max_indent_width = 8;   /* indent stops growing past 8 columns */

    descend(8);

    return 0;
}

/* end ex3d.cpp */
