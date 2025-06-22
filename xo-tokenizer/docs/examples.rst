.. _examples:

.. toctree
   :maxdepth: 2

Examples
========

See ``xo-tokenizer/examples/tokenrepl`` for (slighly elaborated) version of code below

.. code-block:: cpp
   :linenos:

    #include "xo/tokenizer/tokenizer.hpp"

    int
    main() {
        using namespace xo::scm;
        using namespace std;

        using tokenizer_type = tokenizer<char>;
        using span_type = tokenizer_type::span_type;

        tokenizer_type tkz;
        string input_str;

        while (getline(cin, input_str)) {
            // we want tokenizer to see newline, it's syntax
            input_str.push_back('\n');
            span_type input(input_str.begin(), input_str.end());

            // input may contain multiple tokens
            while (!input.empty()) {
                auto [tk, nread] = tkz.scan(input);

                if (tk.is_valid()) {
                    cout << tk;
                }

                input = input.after_prefix(nread);
            }
        }

        auto tk = tkz.notify_eof();

        if (tk.is_valid()) {
            cout << tk;
        }
    }
