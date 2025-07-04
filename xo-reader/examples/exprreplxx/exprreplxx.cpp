/** @file exprreplxx.cpp **/

#include "xo/reader/reader.hpp"
#include <replxx.hxx>
#include <iostream>
#include <unistd.h> // for isatty

// presumeably replxx assumes input is a tty
//
bool replxx_getline(bool interactive, std::size_t parser_stack_size, replxx::Replxx & rx, std::string& input)
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

int
main() {
    using namespace replxx;
    using namespace xo::scm;
    using namespace std;

    using span_type = xo::scm::span<const char>;

    bool interactive = isatty(STDIN_FILENO);

    Replxx rx;
    rx.set_max_history_size(1000);
    rx.history_load("repl_history.txt");
//    rx.bind_key_internal(Replxx::KEY::control('p'), "history_previous");
//    rx.bind_key_internal(Replxx::KEY::control('n'), "history_next");

    reader rdr;
    rdr.begin_interactive_session();

    string input_str;

    bool eof = false;

    span_type input;
    std::size_t parser_stack_size = 0;

    welcome(cerr);

    while (replxx_getline(interactive, parser_stack_size, rx, input_str)) {
        input  = span_type::from_string(input_str);

        while (!input.empty()) {
            auto [expr, consumed, psz] = rdr.read_expr(input, eof);

            if (expr) {
                cout << expr << endl;
            }

            input = input.after_prefix(consumed);
            parser_stack_size = psz;
        }
    }

    auto [expr, _1, _2] = rdr.read_expr(input, true /*eof*/);

    if (expr) {
        cout << expr << endl;
    }

    rx.history_save("repl_history.txt");
}
