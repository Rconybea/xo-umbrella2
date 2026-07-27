/* example ex3c/ex3c.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * xo-ppsink port of xo-indentlog/example/ex3, slice C: function-name styling.
 *
 * XO_ENTER0 captures __PRETTY_FUNCTION__; scope_config::function_style decides
 * how the banner renders it.  Uses a class method so "streamlined" shows
 * "Quadratic::evaluate" (not just "evaluate").  The same call is logged under
 * each style so the difference is visible.
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <iostream>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::log_level;
using xo::pp::tag;
using xo::FunctionStyle;

struct Quadratic {
    int a_, b_, c_;

    /* __PRETTY_FUNCTION__ here is "int Quadratic::evaluate(int) const" */
    int evaluate(int x) const {
        scope log(XO_ENTER0(info), tag("x", x));

        int y = (a_ * x + b_) * x + c_;

        log.end_scope(tag("y", y));

        return y;
    }
};

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;

    Quadratic q{ 1, -3, 2 };   /* x^2 - 3x + 2 */

    for (FunctionStyle style : { FunctionStyle::streamlined,
                                 FunctionStyle::simple,
                                 FunctionStyle::pretty,
                                 FunctionStyle::literal }) {
        scope_config::function_style = style;

        std::clog << "--- function_style = " << descr_of(style) << " ---\n";
        q.evaluate(4);
        std::clog << "\n";
    }

    return 0;
}

/* end ex3c.cpp */
