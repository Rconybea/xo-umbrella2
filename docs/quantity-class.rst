.. _quantity-class:

Quantity
========

.. code-block:: cpp

    #include <xo/unit/quantity.hpp>

The primary data structure for interacting with xo-unit is the
template class ``xo::qty::quantity``.

.. doxygenclass:: xo::qty::quantity

Type Traits
-----------

.. doxygengroup:: quantity-type-traits

Member Variables
----------------

.. doxygengroup:: quantity-static-vars
.. doxygengroup:: quantity-instance-vars

Constructors
------------

.. doxygengroup:: quantity-ctors

The simplest way to create a quantity instance is to use either

*  factory functions in ``xo::qty::qty``, see :doc:`quantity-factoryfunctions`
*  unit variables in ``xo::qty::qty``, see :doc:`quantity-unitvars`

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

.. doxygengroup:: quantity-arithmetic-support

Comparison
----------

Support methods for comparison operators

.. doxygengroup:: quantity-comparison-support
