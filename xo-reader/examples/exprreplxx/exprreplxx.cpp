/** @file exprreplxx.cpp **/

#include "xo/reader/reader.hpp"
#include <replxx.hxx>
#include <iostream>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/expression/pretty_expression.hpp>
#include <unistd.h> // for isatty

// presumeably replxx assumes input is a tty
//
bool replxx_getline(bool interactive,
                    std::size_t parser_stack_size,
                    replxx::Replxx & rx,
                    std::string& input)
{
    using namespace std;

    char const * prompt = "";

    if (interactive) {
        if (parser_stack_size <= 1)
            prompt = "> ";
        else
            prompt = ". ";
    }

    const char * input_cstr = rx.input(prompt);

    bool retval = (input_cstr != nullptr);

    if (retval) {
        //cerr << "got reval->true" << endl;

        input = input_cstr;

    } else {
        //cerr << "got retval->false" << endl;
    }

    rx.history_add(input);

    input.push_back('\n');

    return retval;
}

void
welcome(std::ostream& os)
{
    using namespace std;

    os << "read-eval-print loop for schematika expressions" << endl;
    os << "  ctrl-a/ctrl-e   beginning/end of line" << endl;
    os << "  ctrl-u          delete entire line" << endl;
    os << "  ctrl-k          delete to end of line" << endl;
    os << "  meta-<bs>       backward delete word" << endl;
    os << "  <up>|meta-p     previous command from history" << endl;
    os << "  <down>|meta-n   next command from history" << endl;
    os << "  <pgup>/<pgdown> page through history faster" << endl;
    os << "  ctrl-s/ctrl-r   forward/backward history search" << endl;
    os << endl;
}

/** render @p expr with line breaking, as legacy ppstate_standalone did **/
template <typename T>
static void
render_expr(std::ostream & os, const T & expr) {
    static int seq = 0;

    xo::mm::ArenaConfig logbuf_cfg { .name_ = "exprreplxx." + std::to_string(++seq),
                                     .size_ = 64*1024 };

    xo::pp::PpConfig cfg = xo::pp::PpConfig().with_logbuf_config(logbuf_cfg);

    xo::pp::PrettySink pp(cfg, nullptr);

    pp.pp(expr);

    os << pp.output() << std::endl;
}

int
main()
{
    using namespace replxx;
    using namespace xo::scm;
    using xo::scm::Expression;
    using xo::rp;
    using namespace std;

    using span_type = xo::scm::span<const char>;

    bool interactive = isatty(STDIN_FILENO);

    Replxx rx;
    rx.set_max_history_size(1000);
    rx.history_load("repl_history.txt");
//    rx.bind_key_internal(Replxx::KEY::control('p'), "history_previous");
//    rx.bind_key_internal(Replxx::KEY::control('n'), "history_next");

    constexpr bool c_debug_flag = false;

    rp<GlobalSymtab> toplevel_symtab = GlobalSymtab::make_empty();

    reader rdr(toplevel_symtab, c_debug_flag);
    rdr.begin_interactive_session();

    string input_str;

    bool eof = false;

    span_type input;
    std::size_t parser_stack_size = 0;

    welcome(cerr);

    while (replxx_getline(interactive, parser_stack_size, rx, input_str)) {
        input = span_type::from_string(input_str);

        while (!input.empty()) {
            auto [expr, consumed, psz, error] = rdr.read_expr(input, eof);

            if (expr) {
                render_expr(cout, expr);
            } else if (error.is_error()) {
                cout << "parsing error (detected in " << error.src_function() << "): " << endl;
                error.report(cout);

                /* discard stashed remainder of input line
                 * (for nicely-formatted errors)
                 */
                rdr.reset_to_idle_toplevel();
                break;
            }

            input = input.after_prefix(consumed);
            parser_stack_size = psz;
        }

        /* here: input.empty() or error encountered */

    }

    auto [expr, _1, _2, error] = rdr.read_expr(input, true /*eof*/);

    if (expr) {
        render_expr(cout, rp<Expression>(expr));
    } else if (error.is_error()) {
        cout << "parsing error (detected in " << error.src_function() << "): " << endl;
        error.report(cout);
    }

    rx.history_save("repl_history.txt");
}
