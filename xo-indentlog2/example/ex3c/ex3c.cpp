/* example ex3c/ex3c.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * Like xo-ppsink/example/ex3c (function-name styling), but colored AND routed
 * through the arena-backed PrettySink (from xo-indentlog2) -- so it shows that
 * function styling + color render correctly through the real pretty-print
 * engine, not only the degenerate FlatSink.
 *
 * The color escapes are non-printing (PpState::count_visible_chars skips them),
 * so they don't perturb the engine.  Run in a terminal to see color; piped,
 * the raw \033[...m escapes are visible.
 */

#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/color.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <iostream>
#include <string>

/* This TU also pulls in legacy xo-indentlog (via PrettySink -> xo-arena), which
 * defines XO_ENTER0 / XO_SCOPE.  ppsink's macros are spelled XO_ENTER0_ /
 * XO_SCOPE_ (trailing underscore) precisely so they coexist without collision.
 */

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::tag_config;
using xo::pp::color_config;
using xo::pp::color_spec_type;
using xo::pp::log_level;
using xo::pp::tag;
using xo::pp::PrettySink;
using xo::pp::PpConfig;
using xo::pp::ThreadLogState;
using xo::mm::ArenaConfig;
using xo::FunctionStyle;

struct Quadratic {
    int a_, b_, c_;

    /* __PRETTY_FUNCTION__ here is "int Quadratic::evaluate(int) const" */
    int evaluate(int x) const {
        scope log(XO_ENTER0_(info), tag("x", x));

        int y = (a_ * x + b_) * x + c_;

        log.end_scope(tag("y", y));

        return y;
    }
};

/* run Quadratic::evaluate at function style @p style, capturing the scope
 * output from an arena-backed PrettySink.
 */
static std::string
render(FunctionStyle style) {
    scope_config::function_style = style;

    ArenaConfig logbuf_cfg { .name_ = "example.ex3c", .size_ = 64*1024 };
    PrettySink pp(PpConfig().with_logbuf_config(logbuf_cfg), nullptr /*out*/);

    ThreadLogState::log_set_sink(&pp);
    Quadratic{ 1, -3, 2 }.evaluate(4);
    ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */

    return std::string(pp.output());
}

int
main(int argc, char ** argv) {
    color_config::color_enabled = true;
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;
    scope_config::function_entry_color = color_spec_type::xterm(69);   /* entry: blue-ish */
    scope_config::function_exit_color  = color_spec_type::xterm(70);   /* exit:  green-ish */
    tag_config::tag_color              = color_spec_type::xterm(166);  /* tag name: orange */

    for (FunctionStyle style : { FunctionStyle::streamlined,
                                 FunctionStyle::simple,
                                 FunctionStyle::pretty,
                                 FunctionStyle::literal }) {
        std::cout << "--- function_style = " << descr_of(style) << " (via PrettySink) ---\n"
                  << render(style) << "\n";
    }

    return 0;
}

/* end ex3c.cpp */
