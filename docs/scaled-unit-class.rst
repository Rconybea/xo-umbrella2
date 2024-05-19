.. _scaled-unit-class:

Scaled Unit
===========

.. ditaa::

    +----------------+----------------+
    |    quantity    |    xquantity   |
    +----------------+----------------+
    |cYEL       scaled_unit           |
    +---------------------------------+
    |          natural_unit           |
    +---------------------------------+
    |               bpu               |
    +----------------+                |
    |    bu_store    |                |
    +----------------+----------------+
    |            basis_unit           |
    +---------------------------------+
    |             dimension           |
    +---------------------------------+


.. code-block::cpp

    #include <xo/unit/scaled_unit.hpp>

Result of mutliplication or division of natural units (:doc:`natural-unit-class`).

Motivation
----------

Consider multiplying two units:

.. code-block:: cpp

    using namespace xo::qty;

    constexpr auto u_prod = u::meter * u::kilometer;

How should we represent the product?

We don't want to mix units. Instead we consolidate on a common unit;
to do this we accumulate a product of conversion factors from such consolidation.

For example:

.. code-block:: cpp

    static_assert(u_prod.natural_unit[0].bu() == detail::bu::meter);
    static_assert(u_prod.natural_unit[0].power() == power_ratio_type(2));
    static_assert(u_prod.outer_scale_factor_ == xo::ratio::ratio<int64_t>(1000));
    static_assert(u_prod.outer_scale_sq_ == 1.0);   // used if fractional dimension

Class
-----

.. doxygenclass:: xo::qty::scaled_unit
