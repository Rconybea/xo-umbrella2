.. _natural-unit-class:

Natural Unit
============

.. ditaa::

    +----------------+----------------+
    |    quantity    |    xquantity   |
    +----------------+----------------+
    |           scaled_unit           |
    +---------------------------------+
    |cYEL      natural_unit           |
    +---------------------------------+
    |               bpu               |
    +----------------+                |
    |    bu_store    |                |
    +----------------+----------------+
    |            basis_unit           |
    +---------------------------------+
    |             dimension           |
    +---------------------------------+

.. code-block:: cpp

    #include <xo/unit/natural_unit.hpp>

Representation for the unit associated with a @ref quantity or xquantity.

- represents a cartesian product of basis units.
- constexpr implementation
- limited support for fractional dimensions such as time^-1/2

.. doxygenclass:: xo::qty::natural_unit

.. doxygengroup:: natural-unit-instance-vars
.. doxygengroup:: natural-unit-ctors
.. doxygengroup:: natural-unit-access-methods
.. doxygengroup:: natural-unit-methods
.. doxygengroup:: natural-unit-conversion-methods
