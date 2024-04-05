.. xo-unit-examples documentation master file, created by
   sphinx-quickstart on Wed Mar  6 23:32:27 2024.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

xo-unit documentation
=====================

xo-unit is a lightweight header-only library that provides compile-time
dimension checking and unit conversion.

Functionality is similar in spirit to that provided by ``boost::units``;
however there are some important differences:

First, implementation relies on c++20 features
like class-instance template parameters to efficiently assemble string constants at compile time.

Second, ``xo-unit`` supports fractional dimensions.  This allows using it to naturally handle
concepts like volatility (dimension 1/sqrt(time)), for example.

Finally, ``xo-unit`` is written with the expectation of providing
python integration via pybind11. This requires a parallel set of data structures that can work at
runtime (since we can't construct new c++ types at runtime).

.. toctree::
   :maxdepth: 2
   :caption: xo-unit contents:

   install
   examples
   unit-quantities
   quantity-reference
   unit-reference

Indices and Tables
------------------

* :ref:`glossary`
* :ref:`genindex`
* :ref:`search`
