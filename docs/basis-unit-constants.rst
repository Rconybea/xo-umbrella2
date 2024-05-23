.. _basis-unit-constants:

Basis Unit Constants
====================

Relative scalefactors for each built-in unit.

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+---------------+
    |     quantity   |   xquantity   |
    +----------------+---------------+
    |          scaled_unit           |
    +--------------------------------+
    |         natural_unit           |
    +--------------------------------+
    |               bpu              |
    +----------------+               |
    |    bu_store    |               |
    +----------------+---------------+
    |cBLU       basis_unit           |
    +--------------------------------+
    |            dimension           |
    +--------------------------------+

Introduction
------------

Constants in the :code:`xo::qty::detail::bu` namespace represent
low-level building blocks for specifying units.
Relative scalefactors for each unit are chosen here.

Application code will not typically use these values directtly;
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
