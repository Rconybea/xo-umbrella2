.. _bpu-class:

BPU
===

A rational (usually integral) power of a single basis unit

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
    |cBLU           bpu              |
    +----------------+               |
    |    bu_store    |               |
    +----------------+---------------+
    |           basis_unit           |
    +--------------------------------+
    |            dimension           |
    +--------------------------------+

Introduction
------------

.. code-block:: cpp

    #include <xo/unit/bpu.hpp>

A :code:`xo::qty::bpu` (aka "basis power unit") represents a rational (usually integer) power of a :doc:`basis-unit-class`.

For example:

.. uml::
    :caption: representation for cubic millimeters
    :scale: 99%
    :align: center

    object mm3<<bpu>>
    mm3 : native_dim = dim::distance
    mm3 : scalefactor = 1/1000
    mm3 : power = 3/1

.. uml::
    :caption: representation for annual (365-day) volatility
    :scale: 99%
    :align: center

    object vol<<bpu>>
    vol : native_dim = dim::time
    vol : scalefactor = 365*24*3600
    vol : power = -1/2

:code:`bpu` is intended as an implementation-level abstraction.
Application code will normally interact with the more-general :code:`scaled_unit`
instead of :code:`bpu`.

Class
-----

.. doxygenclass:: xo::qty::bpu

Member Variables
----------------

.. doxygengroup:: bpu-instance-vars

Constructors
------------

.. doxygengroup:: bpu-ctors

Access Methods
--------------

.. doxygengroup:: bpu-access-methods

Other Methods
-------------

.. doxygengroup:: bpu-methods

Comparison
----------

.. doxygengroup:: bpu-comparison

Details
-------

.. doxygengroup:: bpu-abbrev-helpers
