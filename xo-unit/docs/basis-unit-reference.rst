.. _basis-unit-reference:

Basis Unit Reference
====================

Built-in named units for each native dimension

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

.. toctree::
   :maxdepth: 1

   Basis Unit Class <basis-unit-class>
   basis-unit-constants
