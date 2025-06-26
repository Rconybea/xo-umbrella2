.. _examples:

.. toctree::
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
                auto [tk, consumed, error] = tkz.scan(input);

                if (tk.is_valid()) {
                    cout << tk;
                }

                input = input.after_prefix(consumed.size());
            }
        }

        auto [tk, consumed, error] = tkz.notify_eof(spxn_type::from_string(input_str));

        if (tk.is_valid()) {
            cout << tk;
        } else if (error.is_error()) {
            cout << "parsing error: " << endl;
            error.report(cout);
        }
    }

Reminder: enable building examples with ``cmake -DXO_ENABLE_EXAMPLES=1 ..``

.. code-block::
   :linenos:

    $ .build/xo-tokenizer/example/tokenrepl/xo_tokenizer_repl
    > 123
    <token :type tk_i64 :text 123>
    > 123e5
    <token :type tk_f64 :text 123e5>
    > def sq(x: i64) -> i64 { x * x }
    <token :type tk_def :text "">
    <token :type tk_symbol :text sq>
    <token :type tk_leftparen :text "">
    <token :type tk_symbol :text x>
    <token :type tk_colon :text "">
    <token :type tk_symbol :text i64>
    <token :type tk_rightparen :text "">
    <token :type tk_yields :text "">
    <token :type tk_symbol :text i64>
    <token :type tk_leftbrace :text "">
    <token :type tk_symbol :text x>
    <token :type tk_star :text "">
    <token :type tk_symbol :text x>
    <token :type tk_rightbrace :text "">

Example of error reporting (via ``error.report(cout)`` above)

.. code-block::
   :linenos:

    $ .build/xo-tokenizer/example/tokenrepl/xo_tokenizer_repl

    > 123q
    parsing error:
    char: 4
    input: 123q
           ---^
    unexpected character in numeric constant

    > (8 * 8 * 123fd)
    parsing error:
    char: 13
    input: (8 * 8 * 123fd)
                    ---^
    unexpected character in numeric constant
