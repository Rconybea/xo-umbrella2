xo-flatstring documentation master file

xo-flatstring documentation
===========================

xo-flatstring is a lightweight header-only library that provides a constexpr
fixed-size no-allocation string implementation.

Why ``flatstring``?

1.  ``flatstring`` instances can be used as template arguments. [1]_

2.  ``flatstring`` operations (construction, concatenation, ...) are ``constexpr``, so can be done at compile time. [2]_

3.  a ``flatstring`` expression can occupy both compile-time and runtime roles. [3]_

.. [1] A fixed-size char array *can* be used as a template
    argument,  but char* pointers cannot. Automatic conversion of char arrays to pointers in various contexts
    makes them difficult to work with in c++ templates.

.. [2] Although allocation is permitted in constexpr code, it's subject to several restrictions.
    it's not yet possible (as of c++23) to use ``std::string`` at compile time.

.. [3] contrast with a solution relying on template arguments, which must then be compile-time-only.

.. toctree::
   :maxdepth: 2
   :caption: xo-flatstring contents:

   install
   lessons
   flatstring-reference

Indices and Tables
------------------

* :ref:`genindex`
* :ref:`search`
