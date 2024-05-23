.. _bu-store-class:

Basis Unit Store
================

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
    |cBLU bu_store   |               |
    +----------------+---------------+
    |           basis_unit           |
    +--------------------------------+
    |            dimension           |
    +--------------------------------+

.. code-block:: cpp

    #include <xo/unit/bu_store.hpp>

    namespace bu = xo::qty::detail::bu;

A :code:`xo::qty::bu_store` is a small, constexpr, key-value store associating
abbreviations with basis units. To satisfy the constexpr requirement,
all unit abbreviations are irrevocably established from ``bu_store``'s constructor.

The constant ``bu_abbrev_store`` contains a single instance of ``bu_store``,
recording all built-in units along with their associated abbreviations

.. uml::
    :caption: basis-unit store
    :scale: 99%
    :align: center

    map mass_table<<bu_dim_store>> {
    bu::milligram => "mg"
    bu::gram => "g"
    bu::kilogram => "kg"
    }

    map distance_table<<bu_dim_store>> {
    bu::millimeter => "mm"
    bu::meter => "m"
    bu::kilometer => "km"
    }

    map time_table<<bu_dim_store>> {
    bu::millisecond => "ms"
    bu::second => "s"
    bu::minute => "min"
    bu::hour => "hr"
    }

    object bu_abbrev_store<<bu_store>>
    bu_abbrev_store : bu_abbrev_vv[dim::mass] = mass_table
    bu_abbrev_store : bu_abbrev_vv[dim::distance] = distance_table
    bu_abbrev_store : bu_abbrev_vv[dim::time] = time_table

    bu_abbrev_store o-- mass_table
    bu_abbrev_store o-- distance_table
    bu_abbrev_store o-- time_table


This class exists to support the implementation of ``natural_unit::abbrev()``.

Application code is not expected to interact directly with it.

Class
-----

.. doxygenclass:: xo::qty::detail::bu_store

For example, this would be possible:

.. code-block:: cpp

    #include <xo/unit/bu_store.hpp>

    namespace bu = using xo::qty::detail::bu;
    using xo::qty::detail::bu_store;
    using xo::qty::dim;
    using xo::flatstring;

    constexpr bu_store store;
    static_assert(store.bu_abbrev(bu::minute) == flatstring("min"));
    static_assert(store.bu_abbrev(bu::microgram) == flatstring("ug"));

.. doxygengroup:: bu-store-constructors
.. doxygengroup:: bu-store-access-methods
.. doxygengroup:: bu-store-implementation-methods

.. doxygenclass:: xo::qty::detail::bu_dim_store
.. doxygengroup:: bu-dim-store-type-traits
.. doxygengroup:: bu-dim-store-instance-vars


Constants
---------

Provides dictionary of unit abbreviations

Application code is not expected to interact directly with ``bu_abbrev_store``.

.. doxygenvariable:: xo::qty::bu_abbrev_store

Functions
---------

.. doxygenfunction:: xo::qty::bu_abbrev

For example:

.. code-block:: cpp

    #include <xo/unit/bu_store.hpp>

    namespace bu = xo::qty::bu;
    using xo::qty::bu_abbrev;
    using xo::flatstring;

    static_assert(bu_abbrev(bu::kilogram) == xo::flatstring("kg"));
