.. _basis-unit-constants:

Basis Unit Constants
====================

These constants represent low-level building blocks.
Relative scalefactors for each unit are chosen here.

Application code will not use this class directly;
instead it's expected to use units from the :code:`xo::qty::u` namespace.
Those units are implemented on top of the basis units described here.

.. code-block:: cpp

    #include <xo/unit/basis_unit.hpp>

    using xo::qty::detail::bu;

Mass Units
----------

.. doxygengroup:: basis-unit-mass-units

Distance Units
--------------

.. doxygengroup:: basis-unit-distance-units

Time Units
----------

.. doxygengroup:: basis-unit-time-units
