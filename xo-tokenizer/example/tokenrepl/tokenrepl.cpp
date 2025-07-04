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

    while (repl_getline(interactive, cin, cout, input_str)) {
        // we want tokenizer to see newline, it's syntax
        input_str.push_back('\n');
        span_type input = span_type::from_string(input_str);

        // reminder: input may contain multiple tokens
        while (!input.empty()) {
            auto [tk, consumed, error] = tkz.scan(input);

            if (tk.is_valid()) {
                cout << tk << endl;
            } else if (error.is_error()) {
                cout << "parsing error: " << endl;
                error.report(cout);

                break;
            }

            input = tkz.consume(consumed, input);
        }

        /* here: input.empty() or error encountered */

        /* discard stashed remainder of input line
         * (for nicely-formatted errors)
         */
        tkz.discard_current_line();
    }

    {
        span_type input = span_type::from_string(input_str);

        auto [tk, consumed, error] = tkz.notify_eof(input);

        input = tkz.consume(consumed, input);

        if (tk.is_valid()) {
            cout << tk << endl;
        } else if (error.is_error()) {
            cout << "parsing error: " << endl;
            error.report(cout);
        }
    }
}

/** end tokenrepl.cpp */
