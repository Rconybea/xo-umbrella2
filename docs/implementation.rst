.. _implementation:

Abstraction Tower
=================

Abstraction tower for *xo-unit* components.

.. ditaa::

    +-----------+-----------+
    |  quantity | xquantity |
    +-----------+-----------+
    |      scaled_unit      |
    +-----------------------+
    |     natural_unit      |
    +-----------------------+
    |          bpu          |
    +-----------+           |
    |  bu_store |           |
    +-----------+-----------+
    |       basis_unit      |
    +-----------------------+
    |       dimension       |
    +-----------------------+

- quantity: see :doc:`quantity-reference`.
  quantity with compile-time unit work

- xquantity:
  quantity with unit work deferred until runtime

- scaled_unit:
  a unit involving zero or more dimensions, and associated conversion factor.

  - can express result of arithmetic involving multiple scales,
    by reporting an outer scalefactor
  - a scaled unit is 'natural' if its outer scalefactor is 1.
  - quantities are represented by associating a natural scaled_unit instance
  - scaled_units are closed under multiplication and division.
  - multiplication and division commit to a single :code:`basis_unit` for each
    dimension.

- natural_unit:
  a unit involving zero or more dimensions, and at most one scale per dimension.
  A quantity instance is always represented as a dimensionless multiple
  of a natural unit

  - natural_units are *not* closed under multiplication and division.
    (for example consider :code:`xo::qty::qty::foot * xo::qty::qty::meter`)

- bpu: :doc:`bpu-class`.

  A rational (usually integer) power of a basis unit. Has a single dimension.

- bu_store: :doc:`bu-store-class`.

  Associates basis units with abbreviations.

  For example ``bu::kilogram`` => ``"kg"``

- basis_unit: :doc:`basis-unit-reference`.

  A unit with a single dimension and scale.

- dimension: :doc:`dimension-enum`.

  identifies a dimension, such as mass or time.

Representation
==============

Worked example using :cpp:class:`xo::qty::quantity`

.. code-block:: cpp
    :linenos:
    :emphasize-lines: 6

    #include "xo/unit/quantity.hpp"
    ...
    namespace q = xo::qty::qty;

    // 7.55km.min^-2
    quantity qty1 = 7.55 * q::kilometer / (q::minute * q::minute);

.. uml::
    :caption: representation for quantity 7.55km.min^-2
    :scale: 99%
    :align: center

    object qty1<<quantity>>
    qty1 : scale = 7.55

    rectangle {

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

    rectangle {

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

    rectangle {

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

    rectangle {

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
