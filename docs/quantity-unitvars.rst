.. _quantity-unitvars:

.. toctree::
   :maxdepth: 2

Quantity Unit Variables
=======================

.. code-block:: cpp

    #include <xo/unit/quantity.hpp>

The ``xo::unit::unit_qty`` namespace contains unit quantities in each dimension.
Can use these to request unit conversion involving multiple dimensions, for example:

.. code-block:: cpp

    namespace qty = xo::unit::qty;
    namespace u = xo::unit::unit_qty;

    auto q1 = (qty::kilometers(150.0) / qty::hours(0.5);
    auto q2 = q1.with_units_from(u:meter / u:second);


Mass
----
.. doxygenvariable:: xo::unit::unit_qty::milligram
.. doxygenvariable:: xo::unit::unit_qty::gram
.. doxygenvariable:: xo::unit::unit_qty::kilogram

Distance
--------
.. doxygenvariable:: xo::unit::unit_qty::millimeter
.. doxygenvariable:: xo::unit::unit_qty::meter
.. doxygenvariable:: xo::unit::unit_qty::kilometer

Time
----
.. doxygenvariable:: xo::unit::unit_qty::nanosecond
.. doxygenvariable:: xo::unit::unit_qty::microsecond
.. doxygenvariable:: xo::unit::unit_qty::millisecond
.. doxygenvariable:: xo::unit::unit_qty::second
.. doxygenvariable:: xo::unit::unit_qty::minute
.. doxygenvariable:: xo::unit::unit_qty::hour
.. doxygenvariable:: xo::unit::unit_qty::day
