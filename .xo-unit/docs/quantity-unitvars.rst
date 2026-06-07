.. _quantity-unitvars:

Quantity Unit Variables
=======================

Built-in unit quantities

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

Introduction
------------

.. code-block:: cpp

    #include <xo/unit/quantity.hpp>

The ``xo::qty::qty`` namespace contains unit quantities in each dimension.
Can use these to assemble unit quantities with compound dimensions

.. code-block:: cpp
    :linenos:
    :emphasize-lines: 6

    #include "xo/unit/quantity.hpp"

    namespace q = xo::qty::qty;

    auto q1 = (q::kilometers(150.0) / q::hours(0.5));
    constexpr auto u_mps = q:meter / q:second;
    auto q2 = with_units_from(q1, u_mps);


Mass
----
.. doxygenvariable:: xo::qty::qty::picogram
.. doxygenvariable:: xo::qty::qty::nanogram
.. doxygenvariable:: xo::qty::qty::microgram
.. doxygenvariable:: xo::qty::qty::milligram
.. doxygenvariable:: xo::qty::qty::gram
.. doxygenvariable:: xo::qty::qty::kilogram
.. doxygenvariable:: xo::qty::qty::tonne
.. doxygenvariable:: xo::qty::qty::kilotonne
.. doxygenvariable:: xo::qty::qty::megatonne
.. doxygenvariable:: xo::qty::qty::gigatonne

Distance
--------
.. doxygenvariable:: xo::qty::qty::picometer
.. doxygenvariable:: xo::qty::qty::nanometer
.. doxygenvariable:: xo::qty::qty::micrometer
.. doxygenvariable:: xo::qty::qty::millimeter
.. doxygenvariable:: xo::qty::qty::meter
.. doxygenvariable:: xo::qty::qty::kilometer
.. doxygenvariable:: xo::qty::qty::megameter
.. doxygenvariable:: xo::qty::qty::gigameter
.. doxygenvariable:: xo::qty::qty::lightsecond
.. doxygenvariable:: xo::qty::qty::astronomicalunit
.. doxygenvariable:: xo::qty::qty::inch
.. doxygenvariable:: xo::qty::qty::foot
.. doxygenvariable:: xo::qty::qty::yard
.. doxygenvariable:: xo::qty::qty::mile

Time
----
.. doxygenvariable:: xo::qty::qty::picosecond
.. doxygenvariable:: xo::qty::qty::nanosecond
.. doxygenvariable:: xo::qty::qty::microsecond
.. doxygenvariable:: xo::qty::qty::millisecond
.. doxygenvariable:: xo::qty::qty::second
.. doxygenvariable:: xo::qty::qty::minute
.. doxygenvariable:: xo::qty::qty::hour
.. doxygenvariable:: xo::qty::qty::day
.. doxygenvariable:: xo::qty::qty::week
.. doxygenvariable:: xo::qty::qty::month
.. doxygenvariable:: xo::qty::qty::year
.. doxygenvariable:: xo::qty::qty::year250
.. doxygenvariable:: xo::qty::qty::year360
.. doxygenvariable:: xo::qty::qty::year365
