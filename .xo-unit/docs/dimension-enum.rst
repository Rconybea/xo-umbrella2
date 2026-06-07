.. _dimension:

Native Dimension
================

An abstract dimension;  distinct native dimensions are orthogonal

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
    |           basis_unit           |
    +--------------------------------+
    |cBLU        dimension           |
    +--------------------------------+

Introduction
------------

Identifies an abstract dimension, for example *mass* or *time*.

.. code-block:: cpp

    #include <xo/unit/dimension.hpp>

For example can use this enum to index basis members of a :doc:`scaled_unit<scaled-unit-class>` instance:

.. code-block:: cpp
    :emphasize-lines: 7-8

    #include <xo/unit/quantity.hpp>

    using namespace xo::qty;

    auto x = q::kilometers(100) / q::hours(1);

    auto bpu1 = x.lookup_dim(dim::time);
    auto bpu2 = x.lookup_dim(dim::distance);

Enum
----

.. doxygenenum:: xo::qty::dimension

Constants
---------

.. doxygenvariable:: xo::qty::n_dim

Functions
---------

.. doxygenfunction:: xo::qty::dim2str
