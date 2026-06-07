.. _natural-unit-class:

Natural Unit
============

A natural unit represents a product of terms, each involving a distinct basis dimension

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+---------------+
    |     quantity   |   xquantity   |
    +----------------+---------------+
    |          scaled_unit           |
    +--------------------------------+
    |cBLU     natural_unit           |
    +--------------------------------+
    |               bpu              |
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

    #include <xo/unit/natural_unit.hpp>

Representation for the unit associated with a :doc:`xquantity<xquantity-class>`

- represents a cartesian product of basis units.
- constexpr implementation
- limited support for fractional dimensions such as time^-1/2

.. uml::
    :caption: natural unit for a Newton (unit of force)
    :scale: 99%
    :align: center

    object newton<<natural_unit>>
    newton : n_bpu = 3
    newton : bpu_v[]

    object kg<<bpu>>
    kg : native_dim = dim::mass
    kg : scalefactor = 1000/1
    kg : power = 1/1

    object m<<bpu>>
    m : native_dim = dim::distance
    m : scalefactor = 1/1
    m : power = 1/1

    object s2<<bpu>>
    s2 : native_dim = dim::time
    s2 : scalefactor = 1/1
    s2 : power = -2/1

    newton o-- kg
    newton o-- m
    newton o-- s2

Class
-----

.. doxygenclass:: xo::qty::natural_unit

Member Variables
----------------

.. doxygengroup:: natural-unit-instance-vars

Type Traits
-----------

.. doxygengroup:: natural-unit-type-traits

Constructors
------------

.. doxygengroup:: natural-unit-ctors

Access Methods
--------------

.. doxygengroup:: natural-unit-access-methods

General Methods
---------------

.. doxygengroup:: natural-unit-methods

Conversion
----------

.. doxygengroup:: natural-unit-conversion-methods

Comparison Functions
--------------------

.. doxygengroup:: natural-unit-comparison-functions
