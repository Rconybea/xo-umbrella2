/** @file exprrepl.cpp **/

#include "xo/reader/reader.hpp"
#include <iostream>
#include <unistd.h> // for isatty

bool repl_getline(bool interactive, std::istream& in, std::ostream& out, std::string& input)
{
    if (interactive) {
        out << "> ";
        std::flush(out);
    }

    bool retval = static_cast<bool>(std::getline(in, input));

    if (retval) {
        // want reader to see newline, it's syntax
        input.push_back('\n');
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

    while (repl_getline(interactive, cin, cout, input_str)) {
        input  = span_type::from_string(input_str);

        while (!input.empty()) {
            auto [expr, consumed] = rdr.read_expr(input, eof);

            if (expr) {
                cout << expr << endl;
            }

            input = input.after_prefix(consumed);
        }
    }

    auto [expr, _] = rdr.read_expr(input, true /*eof*/);

    if (expr) {
        cout << expr << endl;
    }
}
