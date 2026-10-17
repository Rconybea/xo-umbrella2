.. _coding-style:

Coding Style
============

Conventions for C++ source across the XO subsystems.

Qualifying Namespaced Symbols
-----------------------------

Two extremes are both costly:

- **Fully qualifying every use** (``xo::json::json_open{..}``, the same
  qualifier repeated for the same symbol) is verbose, and buries the lede.
  A reader must parse qualification noise that take focus away from
  more important content.
- **``using namespace xo::foo;``** breaks divide-and-conquer: a reader cannot
  know how a symbol resolves without reading every header included before
  it.  We avoid it.

Instead we write individual ``using`` declarations, each naming one fully
qualified symbol, and collect them in one place per scope:

- **In a .cpp file**: together, inside the top-level ``namespace xo {``
  (``namespace ut {`` in a unit test).

  .. code-block:: cpp

     namespace xo {
         using xo::json::PrintJson;
         using xo::json::json_open;
         using xo::pp::PpSink;

         ...  // bare names from here on
     }

- **At class scope**: type aliases, together at the top of the class.  We
  use these freely, to keep method bodies free of qualifiers.

  .. code-block:: cpp

     class EigenVectorJsonPrinter : public JsonPrinter {
     public:
         using PpSink = xo::pp::PpSink;
         ...
     };

- **In a function or method defined in a .hpp file**: ``using``
  declarations at the top of the function body.  Never at namespace scope
  in a header: there they leak into every file that includes it.

  .. code-block:: cpp

     static void print(PpSink & sink, json_close const & x) {
         using xo::json::c_json_indent;

         sink.split(0, -c_json_indent);
         ...
     }

Why collect them: "where does ``foo`` come from?" is usually a question of
low value.  Answering it once, in one place, keeps it from competing for
attention with the code that matters.  It also keeps provenance local: a
symbol's origin is always stated either in the same .cpp file as its use,
or in the .hpp file that defines the type bearing it.

Where C++ requires a qualified name -- the argument of a template
specialization at namespace scope, say ``Prettifier<xo::json::json_open>``
-- we write it qualified.

Includes
--------

Brackets
~~~~~~~~

- **Same directory**: ``#include "foo.hpp"``.
- **Elsewhere in the same subsystem** (an ancestor directory, say):
  ``#include "xo/sub/foo.hpp"``.  Not a bare ``"foo.hpp"``: gcc resolves
  that through the including file's directory chain, clang does not, and an
  installed (satellite) build has only root-level ``-I`` paths.
- **Another subsystem**: ``#include <xo/other/foo.hpp>``.
- never use ``../`` to reach a header, even within the same subsystem.

Order
~~~~~

1. the file's own header (in a .cpp)
2. the same subsystem's headers
3. other XO subsystems, in **descending** dependency order -- a subsystem
   before the subsystems it depends on
4. third-party libraries (e.g. Catch2, Eigen)
5. the C++ standard library
6. platform headers

The dependency order for step 3 is generated: the umbrella configure writes
``.build/dot-clang-format``, whose ``IncludeCategories`` give each
``<xo/SUB/`` a priority (lower sorts first).  Look the subsystems up there
rather than guessing:

.. code-block:: console

   $ grep -n "xo/indentlog2/\|xo/ppsink/" .build/dot-clang-format

Within one subsystem's group, order is free: dependency order is the rule;
alphabetical order is not.
