.. _quantity_factoryfunctions:

Quantity Factory Functions
==========================

Convenience functions for creating quantities with compile-time units

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+---------------+
    |cBLU quantity   |   xquantity   |
    +----------------+---------------+
    |          scaled_unit           |
    +--------------------------------+
    |         natural_unit           |
    +--------------------------------+
    |               bpu              |
    +----------------+               |
    |    bu_store    |               |
    +----------------+---------------+
    |           basis_unit           |
    +--------------------------------+
    |            dimension           |
    +--------------------------------+

.. toctree::
   :maxdepth: 2

Introduction
------------

.. code-block:: cpp

    #include <xo/unit/quantity.hpp>

Mass
----
.. doxygenfunction:: xo::qty::qty::picograms
.. doxygenfunction:: xo::qty::qty::nanograms
.. doxygenfunction:: xo::qty::qty::micrograms
.. doxygenfunction:: xo::qty::qty::milligrams
.. doxygenfunction:: xo::qty::qty::grams
.. doxygenfunction:: xo::qty::qty::kilograms
.. doxygenfunction:: xo::qty::qty::tonnes
.. doxygenfunction:: xo::qty::qty::kilotonnes
.. doxygenfunction:: xo::qty::qty::megatonnes
.. doxygenfunction:: xo::qty::qty::gigatonnes

Distance
--------
.. doxygenfunction:: xo::qty::qty::picometers
.. doxygenfunction:: xo::qty::qty::nanometers
.. doxygenfunction:: xo::qty::qty::micrometers
.. doxygenfunction:: xo::qty::qty::millimeters
.. doxygenfunction:: xo::qty::qty::meters
.. doxygenfunction:: xo::qty::qty::kilometers
.. doxygenfunction:: xo::qty::qty::megameters
.. doxygenfunction:: xo::qty::qty::gigameters

.. doxygenfunction:: xo::qty::qty::lightseconds
.. doxygenfunction:: xo::qty::qty::astronomicalunits

.. doxygenfunction:: xo::qty::qty::inches
.. doxygenfunction:: xo::qty::qty::feet
.. doxygenfunction:: xo::qty::qty::yards
.. doxygenfunction:: xo::qty::qty::miles

Time
----
.. doxygenfunction:: xo::qty::qty::picoseconds
.. doxygenfunction:: xo::qty::qty::nanoseconds
.. doxygenfunction:: xo::qty::qty::microseconds
.. doxygenfunction:: xo::qty::qty::milliseconds
.. doxygenfunction:: xo::qty::qty::seconds
.. doxygenfunction:: xo::qty::qty::minutes
.. doxygenfunction:: xo::qty::qty::hours
.. doxygenfunction:: xo::qty::qty::days
.. doxygenfunction:: xo::qty::qty::weeks
.. doxygenfunction:: xo::qty::qty::months
.. doxygenfunction:: xo::qty::qty::years
.. doxygenfunction:: xo::qty::qty::year250s
.. doxygenfunction:: xo::qty::qty::year360s
.. doxygenfunction:: xo::qty::qty::year365s

Volatility
----------
.. doxygenfunction:: xo::qty::qty::volatility_30d
.. doxygenfunction:: xo::qty::qty::volatility_250d
.. doxygenfunction:: xo::qty::qty::volatility_360d
.. doxygenfunction:: xo::qty::qty::volatility_365d
