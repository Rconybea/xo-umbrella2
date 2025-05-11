.. _scaled-unit-class:

Scaled Unit
===========

A dimensionless multiple of a :doc:`natural_unit<natural-unit-class>`

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+----------------+
    |    quantity    |    xquantity   |
    +----------------+----------------+
    |cBLU       scaled_unit           |
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

Introduction
------------

.. code-block::cpp

    #include <xo/unit/scaled_unit.hpp>

Extension of :doc:`natural_unit<natural-unit-class>` to enable representing the intermediate
result of multiplication (or division) of natural units.

- represents a (dimensionless) multiple of a cartesian product of basis units.
- constexpr implementation
- limited support for fractional dimensions such as time^-1/2

.. uml::
    :caption: scaled unit after (u::meter * u::foot / u::minute)
    :scale: 99%
    :align: center

    object area_per_time<<scaled_unit>>
    area_per_time : outer_scale_factor = 3048/10000
    area_per_time : outer_scale_sq = 1.0
    area_per_time : natural_unit = m2_per_min

    object m2_per_min<<natural_unit>>
    m2_per_min : n_bpu = 2
    m2_per_min : bpu_v[]

    object m2<<bpu>>
    m2 : native_dim = dim::distance
    m2 : scalefactor = 1/1
    m2 : power = 2/1

    object min<<bpu>>
    min : native_dim = dim::time
    min : scalefactor = 60/1
    min : power = -1/1

    area_per_time o-- m2_per_min
    m2_per_min o-- m2
    m2_per_min o-- min

Scaled units with non-unity outer scalefactors arise as intermediate results
of quantity arithmetic

Motivation
^^^^^^^^^^

Consider multiplying two units:

.. code-block:: cpp

    using namespace xo::qty;

    constexpr auto u_prod = u::meter * u::kilometer;

How should we represent the product?

We don't want to mix units. Instead we consolidate on a common unit;
to do this we accumulate a product of conversion factors from such consolidation.

For example:

.. code-block:: cpp
    :emphasize-lines: 3

    static_assert(u_prod.n_bpu() == 1);
    static_assert(u_prod[0].bu() == detail::bu::meter);
    static_assert(u_prod[0].power() == power_ratio_type(2));
    static_assert(u_prod.outer_scale_factor_ == xo::ratio::ratio<int64_t>(1000));
    static_assert(u_prod.outer_scale_sq_ == 1.0);   // used if fractional dimension

Here we accumulate :code:`1000`, from converting kilometers to meters.

Division works similarly. In this example dimension cancel, but we still have a non-unity conversion factor.

.. code-block:: cpp
    :emphasize-lines: 7

    namespace u = xo::qty::u;

    constexpr auto u_div = u::meter / u::kilometer;

    // dimensionlesss result
    static_assert(u_prod.n_bpu() == 0);
    static_assert(u_prod.outer_scale_factor_ == xo::ratio::ratio<int64_t>(1,1000));
    static_assert(u_prod.outer_scale_sq_ == 1.0);

When multiple dimensions needing conversion are involved, scalefactors accumulate:

.. code-block:: cpp
    :emphasize-lines: 8

    namespace u = xo::qty::u;

    constexpr auto u2_prod = u::meter * u::hour * u::kilometer * u::minute;

    static_assert(u2_prod.n_bpu() == 2);
    static_assert(u2_prod[0].bu() == detail::bu::meter);
    static_assert(u2_prod[1].bu() == detail::bu::hour);
    static_assert(u2_prod.outer_scale_factor_ == xo::ratio::ratio<int64_t>(50,3));
    static_assert(u2_prod.outer_scale_sq_ == 1.0);   // used if fractional dimension

Here the :code:`50/3` result comes from multiplying :code:`1000/1` (converting kilometers -> meters)
by :code:`1/60` (converting minutes -> hours)


Class
-----

.. doxygenclass:: xo::qty::scaled_unit

Member Variables
----------------

.. doxygengroup:: scaled-unit-instance-vars

Type Traits
-----------

.. doxygengroup:: scaled-unit-type-traits

Access Methods
--------------

.. doxygengroup:: scaled-unit-access-methods

General Methods
---------------

.. doxygengroup:: scaled-unit-general-methods

Operators
---------

.. doxygengroup:: scaled-unit-operators
