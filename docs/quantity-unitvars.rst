.. _quantity-unitvars:

.. toctree::
   :maxdepth: 2

Quantity Unit Variables
=======================

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
.. doxygenvariable:: xo::qty::qty::milligram
.. doxygenvariable:: xo::qty::qty::gram
.. doxygenvariable:: xo::qty::qty::kilogram
.. doxygenvariable:: xo::qty::qty::tonne
.. doxygenvariable:: xo::qty::qty::kilotonne

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

Time
----
.. notyet .. doxygenvariable:: xo::qty::qty::picosecond
.. notyet .. doxygenvariable:: xo::qty::qty::nanosecond
.. notyet .. doxygenvariable:: xo::qty::qty::microsecond
.. notyet .. doxygenvariable:: xo::qty::qty::millisecond
.. doxygenvariable:: xo::qty::qty::second
.. doxygenvariable:: xo::qty::qty::minute
.. doxygenvariable:: xo::qty::qty::hour
.. doxygenvariable:: xo::qty::qty::day
.. notyet  doxygenvariable:: xo::qty::qty::week
.. notyet  doxygenvariable:: xo::qty::qty::month
.. notyet  doxygenvariable:: xo::qty::qty::year
.. notyet  doxygenvariable:: xo::qty::qty::year250
.. notyet  doxygenvariable:: xo::qty::qty::year360
.. notyet  doxygenvariable:: xo::qty::qty::year365
