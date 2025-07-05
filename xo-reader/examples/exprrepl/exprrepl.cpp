/** @file exprrepl.cpp **/

#include "xo/reader/reader.hpp"
#include <iostream>
#include <unistd.h> // for isatty

bool repl_getline(bool interactive, std::size_t parser_stack_size, std::istream& in, std::ostream& out, std::string& input)
{
    using namespace std;

    if (interactive) {
        if (parser_stack_size <= 1)
            out << "> ";
        else
            out << ". ";
        std::flush(out);
    }

    bool retval = static_cast<bool>(getline(in, input));

    if (retval) {
        cerr << "got reval->true" << endl;

        // interactive only: treat ^@ (C-RET) as ;RET
        if ((input.size() > 0) && ((*input.rbegin()) == '\0'))
        {
            cerr << "got ^@" << endl;

            *input.rbegin() = ';';
        }

        // want reader to see newline, it's syntax
        input.push_back('\n');
    } else {
        cerr << "got retval->false" << endl;
    }

    return retval;
}

int
main() {
    using namespace xo::scm;
    using namespace std;

    using span_type = xo::scm::span<const char>;

    bool interactive = isatty(STDIN_FILENO);

    reader rdr;
    rdr.begin_interactive_session();

    string input_str;

    bool eof = false;

    span_type input;
    std::size_t parser_stack_size = 0;

    while (repl_getline(interactive, parser_stack_size, cin, cout, input_str)) {
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
}
