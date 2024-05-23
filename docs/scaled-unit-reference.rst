.. _scaled-unit-reference:

Scaled Unit Reference
=====================

A dimensionless multiple of a :doc:`natural_unit<natural-unit-class>`

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

.. toctree::
   :maxdepth: 1

   Scaled Unit Class <scaled-unit-class>
   scaled-unit-constants
