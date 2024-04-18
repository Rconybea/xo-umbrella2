.. _quantity-class:

Quantity
========

.. code-block:: cpp

    #include <xo/unit/quantity.hpp>

The primary data structure for interacting with xo-unit is the
template class ``xo::unit::quantity``.

.. doxygenclass:: xo::unit::quantity

Type Traits
-----------

.. doxygengroup:: quantity-traits
   :content-only:

Constructors
------------

.. doxygengroup:: quantity-ctors
   :content-only:

.. doxygengroup:: quantity-named-ctors
   :content-only:

The simplest way to create a quantity instance is to use either

*  factory functions in ``xo::unit::qty``, see :doc:`quantity-factoryfunctions`
*  unit variables in ``xo::unit::units``, see :doc:`quantity-unitvars`

Assignment
----------

.. doxygengroup:: quantity-assignment
   :content-only:

Access Methods
--------------

.. doxygengroup:: quantity-access-methods
   :content-only:

Constants
---------

.. doxygengroup:: quantity-constants
   :content-only:

Conversion Methods
------------------

Amount-preserving conversion to quantities with different units and/or representation.

.. doxygengroup:: quantity-unit-conversion
   :content-only:

Arithmetic
----------

.. doxygengroup:: quantity-arithmetic
   :content-only:

Support methods for arithmetic operations

.. doxygengroup:: quantity-arithmeticsupport
   :content-only:

Comparison
----------

Support methods for comparison operators

.. doxygengroup:: quantity-comparisonsupport
   :content-only:
