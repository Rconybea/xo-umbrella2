.. _basis-unit-class:

Basis Unit
==========

A unit representing a fixed multiple of a native dimension.

Context
-------

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

Introduction
------------

A :code:`basis_unit` represents a unit belonging to a single native dimension.
For example :code:`bu::meter` representing a distance of 1 meter.

.. code-block:: cpp

    #include <xo/unit/basis_unit.hpp>

.. uml::
    :scale: 99%
    :align: center
    :caption: basis unit representing 1 minute

    object bu1<<basis_unit>>
    bu1 : native_dim = time
    bu1 : scalefactor = 60

:code:`basis_unit` is intended as an implementation-level abstraction.
Application code will normally interact with the more-capable :code:`scaled_unit`
instead of :code:`basis_unit`.

Class
-----

.. doxygenclass:: xo::qty::basis_unit

Member Variables
----------------

.. doxygengroup:: basis-unit-instance-vars

Constructors
------------

.. doxygengroup:: basis-unit-constructors

Access Methods
--------------

.. doxygengroup:: basis-unit-access-methods

Comparison
----------

.. doxygengroup:: basis-unit-comparison-support
