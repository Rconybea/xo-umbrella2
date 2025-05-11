.. _implementation:

Components
==========

Library dependency tower for *xo-unit*:

.. ditaa::

    +-----------------+
    |     xo_unit     |
    +-----------------+
    |    xo_ratio     |
    +-----------------+
    |  xo_flatstring  |
    +-----------------+
    |    xo_cmake     |
    +-----------------+

Install instructions :doc:`here<install>`

Abstraction tower for *xo-unit* components:

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
    |           basis_unit           |
    +--------------------------------+
    |            dimension           |
    +--------------------------------+

- :doc:`quantity<quantity-reference>`:

  A quantity with unit checking and conversion done at compile-time

    .. code-block:: cpp

        #include "xo/unit/quantity.hpp"
        auto q1 = xo::qty::qty::kilometers(7.5);

- :doc:`xquantity<xquantity-reference>`:

  A quantity with unit checking and conversion done at run-time.
  This is useful if unit information isn't known at compile time, for example
  if reading units from console input.

    .. code-block:: cpp

        #include "xo/unit/xquantity.hpp"
        xquantity qty1(7.5, xo::qty::u::foot)


- :doc:`scaled_unit<scaled-unit-class>`:

    .. code-block:: cpp

        #include "xo/unit/scaled_unit.hpp"
        auto u = xo::qty::u::millimeter;

  A unit involving zero or more dimensions, and associated conversion factor.

  - can express result of arithmetic involving multiple scales,
    by reporting an outer scalefactor
  - a scaled unit is 'natural' if its outer scalefactor is 1.
  - quantities are represented by associating a natural scaled_unit instance
  - scaled_units are closed under multiplication and division.
  - multiplication and division commit to a single :code:`basis_unit` for each
    dimension.

- :doc:`natural_unit<natural-unit-class>`

    .. code-block:: cpp

        #include "xo/unit/natural_unit.hpp"
        auto u = xo::qty::nu::millimeter;

  A unit involving zero or more dimensions, and at most one scale per dimension.
  A quantity instance is always represented as a dimensionless multiple
  of a natural unit

  - natural_units are *not* closed under multiplication and division.
    (for example consider :code:`xo::qty::qty::foot * xo::qty::qty::meter`)

- :doc:`bpu<bpu-class>`

  A rational (usually integer) power of a basis unit. Has a single dimension.

    .. code-block:: cpp

        #include "xo/unit/bpu.hpp"
        xo::qty::bpu(xo::qty::detail::bu::millimeter,
                     xo::qty::power_ratio_type(2));  // mm^2

- :doc:`bu_store<bu-store-class>`

  Associates basis units with abbreviations.
  Abbreviations used to decorate printed quantities.

  For example ``bu::kilogram`` => ``"kg"``

    .. code-block:: cpp

        #include "xo/unit/bu_store.hpp"
        xo::qty::bu_abbrev_store.bu_abbrev(xo::qty::detail::bu::picogram); // "pg"

- :doc:`basis_unit<basis-unit-reference>`

  A unit with a single dimension and scale.

    .. code-block:: cpp

        #include "xo/unit/basis_unit.hpp"
        auto b = xo::qty::detail::bu::picogram;

- :doc:`dimension<dimension-enum>`

  identifies a dimension, such as mass or time.

    .. code-block:: cpp

        #include "xo/unit/dimension.hpp"
        auto d = xo::qty::dimension::mass;

Representation
==============

Worked example using :cpp:class:`xo::qty::quantity`.

.. code-block:: cpp
    :linenos:
    :emphasize-lines: 6

    #include "xo/unit/quantity.hpp"
    ...
    using xo::qty;
    namespace q = xo::qty::qty;

    // 7.55km.min^-2
    quantity qty1 = 7.55 * q::kilometer / (q::minute * q::minute);

Note: in diagrams below, components with pale blue background are discarded before runtime

.. uml::
    :caption: representation for quantity 7.55km.min^-2
    :scale: 99%
    :align: center

    object qty1<<quantity>>
    qty1 : scale = 7.55

    rectangle #e0f0ff {

    object km_per_min2<<natural_unit>>
    km_per_min2 : n_bpu = 2
    km_per_min2 : bpu[0] = km
    km_per_min2 : bpu[1] = per_min

    object km<<bpu>>
    km : native_dim = dim.mass
    km : scalefactor = 1000/1
    km : power = 1

    object per_min2<<bpu>>
    per_min2 : native_dim = dim.time
    per_min2 : scalefactor = 60/1
    per_min2 : power = -2

    qty1 o-- km_per_min2 : s_unit (static constexpr)

    km_per_min2 *-- km
    km_per_min2 *-- per_min2

    }

.. code-block:: cpp
   :linenos:

   // 123ng
   quantity qty2 = q::nanograms(123);

.. uml::
    :caption: representation for quantity 123 nanograms
    :scale: 99%
    :align: center

    object qty2<<quantity>>
    qty2 : scale = 123

    rectangle #e0f0ff {

    object ng_unit<<natural_unit>>
    ng_unit : n_bpu = 1
    ng_unit : bpu[0] = ng

    object ng<<bpu>>
    ng : native_dim = dim::mass
    ng : scalefactor = 1/10^9
    ng : power = 1

    qty2 o-- ng_unit : s_unit (static constexpr)

    ng_unit *-- ng

    }

.. code-block:: cpp
    :linenos:

    // (123*7.55) ng.km.min^-2
    quantity qty3 = qty2 * qty1;

.. uml::
    :caption: quantity 928.65 ng.km.min^-2
    :scale: 99%
    :align: center

    object qty3<<quantity>>
    qty3 : scale = 928.65

    rectangle #e0f0ff {

      object ng_km_min2_unit<<natural_unit>>
      ng_km_min2_unit : n_bpu = 3
      ng_km_min2_unit : bpu[0] = ng
      ng_km_min2_unit : bpu[1] = km
      ng_km_min2_unit : bpu[2] = per_min2

      object ng<<bpu>>
      ng : native_dim = dim::mass
      ng : scalefactor = 1/10^9
      ng : power = 1

      object km<<bpu>>
      km : native_dim = dim::distance
      km : scalefactor = 1000/1
      km : power = 1

      object per_min2<<bpu>>
      per_min2 : native_dim = dim::time
      per_min2 : scalefactor = 60/1
      per_min2 : power = -2

      qty3 o-- ng_km_min2_unit : s_unit (static constexpr)

      ng_km_min2_unit *-- ng
      ng_km_min2_unit *-- km
      ng_km_min2_unit *-- per_min2
    }

.. code-block:: cpp
    :linenos:

    namespace u = xo::qty::u;

    // (123*7.55) ng.km.min^-2  ==>  2.57958e-10kg.m.s^-2

    constexpr auto newton = u::kilogram * u::meter / (u::second * u::second);

    quantity<newton> qty3b = qty3;

    // quantity qty3b = qty3.rescale_ext<newton>();

.. uml::
    :caption: quantity 928.65 ng.km.min^-2
    :scale: 99%
    :align: center

    object qty3b<<quantity>>
    qty3b : scale = 2.59758e-10

    rectangle #e0f0ff {

      object kg_m_s2_unit<<natural_unit>>
      kg_m_s2_unit : n_bpu = 3
      kg_m_s2_unit : bpu[0] = kg
      kg_m_s2_unit : bpu[1] = m
      kg_m_s2_unit : bpu[2] = per_s2

      object kg<<bpu>>
      kg : native_dim = dim::mass
      kg : scalefactor = 1000/1
      kg : power = 1

      object m<<bpu>>
      m : native_dim = dim::distance
      m : scalefactor = 1/1
      m : power = 1

      object per_s2<<bpu>>
      per_s2 : native_dim = dim::time
      per_s2 : scalefactor = 1/1
      per_s2 : power = -2

      qty3b o-- kg_m_s2_unit : s_unit (static constexpr)

      kg_m_s2_unit *-- kg
      kg_m_s2_unit *-- m
      kg_m_s2_unit *-- per_s2
    }

.. toctree::
   :maxdepth: 2
   :caption: Abstractions
