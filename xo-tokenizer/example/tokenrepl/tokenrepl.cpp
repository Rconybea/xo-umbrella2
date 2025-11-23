/** @file tokenrepl.cpp **/

#include "xo/tokenizer/tokenizer.hpp"
#include <iostream>
#include <unistd.h> // for isatty

bool repl_getline(bool interactive, std::istream& in, std::ostream& out, std::string& input)
{
    if (interactive) {
        out << "> ";
        std::flush(out);
    }

    return static_cast<bool>(std::getline(in, input));
}

int
main() {
    using namespace xo::scm;
    using namespace std;

    using tokenizer_type = tokenizer<char>;
    using span_type = tokenizer_type::span_type;

    xo::log_config::min_log_level = xo::log_level::info;

    bool interactive = isatty(STDIN_FILENO);

    tokenizer_type tkz(xo::log_config::min_log_level <= xo::log_level::info);
    string input_str;

    size_t line_no = 1;

    constexpr std::size_t c_maxlines = 25;

    while (repl_getline(interactive, cin, cout, input_str)) {
        // we want tokenizer to see newline, it's syntax
        input_str.push_back('\n');
        span_type input = span_type::from_string(input_str);

        // reminder: input may contain multiple tokens
        while (!input.empty()) {
            auto [tk, consumed, error] = tkz.scan(input, false /*!eof*/);

            if (tk.is_valid()) {
                cout << tk << endl;
            } else if (error.is_error()) {
                cout << "tokenizer error: " << endl;
                error.report(cout);

                break;
            }

            input = input.after_prefix(consumed);
        }

        /* here: input.empty() or error encountered */

        ++line_no;

        if (line_no > c_maxlines) {
            cout << "always exit after " << c_maxlines << " lines of input" << endl;
            break;
        }
    }
}

/** end tokenrepl.cpp */
