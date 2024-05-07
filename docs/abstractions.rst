.. _abstractions:

Representation
==============

.. ditaa::

    +-----------+-----------+
    |  quantity | xquantity |
    +-----------+-----------+
    |      scaled_unit      |
    +-----------------------+
    |     natural_unit      |
    +-----------------------+
    |          bpu          |
    +-----------------------+
    |       basis_unit      |
    +-----------------------+
    |       dimension       |
    +-----------------------+

- quantity: see :doc:`quantity-reference`

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

    namespace su = xo::qty::su;

    // (123*7.55) ng.km.min^-2  ==>  2.57958e-10kg.m.s^-2

    quantity qty3b = qty3.rescale_ext<su::kilogram * su::meter / (su::second * su::second)>();

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
