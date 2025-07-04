.. _examples:

.. toctree::
   :maxdepth: 2

Examples
========

See ``xo-reader/examples`` for built examples

.. code-block:: cpp
   :linenos:

    #include "xo/reader/reader.h"

    int
    main() {
        using namespace xo::scm;
        using namespace std;

        reader rdr;
        rdr.begin_translation_unit();

        bool eof = false;
        while (!eof) {
            auto input = ins.read_some();

            eof = ins.eof();

            for (auto rem = input; !rem.empty();) {
                // res: (parsed-expr, used)
                auto [expr, rem2] = rdr.read_expr(rem, eof);

                if (expr) {
                     cout << expr << endl;
                }

                rem = rem.suffix_after(rem2);
            }
        }

        if (rdr.has_prefix()) {
            cout << "error: unparsed input after expression" << endl;
        }
    }
