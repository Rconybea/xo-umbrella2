
xo-ratio documentation
======================

xo-ratio is a lightweight header-only library that provides a constexpr
fixed-size no-allocation ratio implementation.

Why ``ratio``?

1.  ``ratio`` instances can be used as template arguments. [1]_

2.  ``ratio`` operations (arithmetic, integer powers, string conversion, ...) are ``constexpr``, so can be done at compile time. [2]_

3.  a ``ratio`` expression can occupy both compile-time and runtime roles. [3]_

.. [1] verified with gcc 13.2.0;  clang 18 does not recognize ratio<T> as a structured type for some reason.

.. [2] Although allocation is permitted in constexpr code, it's subject to several restrictions.
    it's not yet possible (as of c++23) to use ``std::string`` at compile time.
    Using ``xo::flatstring`` instead.

.. [3] contrast with a solution relying on template arguments, which must then be compile-time-only.

.. toctree::
   :maxdepth: 2
   :caption: xo-ratio contents:

   install
   ratio-reference

Indices and Tables
------------------

* :ref:`genindex`
* :ref:`search`
