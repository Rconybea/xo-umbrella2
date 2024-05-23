.. _scaled-unit-constants

Scaled Unit Constants
=====================

Built-in unit constants, for use with conversions

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+---------------+
    |     quantity   |   xquantity   |
    +----------------+---------------+
    |cBLU      scaled_unit           |
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

Built-in units. Application code may use these to trigger conversion

.. code-block:: cpp
    :emphasize-lines: 5

    #include <xo/unit/quantity.hpp>

    using namespace xo::qty;

    constexpr quantity<u::meter / u::second> q1 = q::miles(60) / q::hour;

Note that it's often easiest to use :doc:`unit quantity constants<quantity-unitvars>`,
like :code:`q::hour` in the example above

Dimensionless Constant
----------------------

.. doxygengroup:: scaled-unit-dimensionless

Mass Units
----------

.. doxygengroup:: scaled-unit-mass

Distance Units
--------------

.. doxygengroup:: scaled-unit-distance

Time Units
----------

.. doxygengroup:: scaled-unit-time

Volatility Units
----------------

.. doxygengroup:: scaled-unit-volatility

Miscellaneous Units
-------------------

.. doxygengroup:: scaled-unit-misc
