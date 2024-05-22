.. _quantity-class:

Quantity
========

.. ditaa::

    +----------------+---------------+
    |cYEL quantity   |   xquantity   |
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
    |            dimension           |
    +--------------------------------+

.. code-block:: cpp

    #include <xo/unit/quantity.hpp>

.. uml::
    :scale: 99%
    :align: center

    allowmixing

    object qty1<<quantity>>
    qty1 : scale = 1.23

    rectangle constexpr #e0f0ff {

    object unit<<natural_unit>>

    qty1 o-- unit : s_unit (static constexpr)

    }

Arithmetic on :doc:`xo::qty::quantity<quantity-reference>` instances
does *not* use ``xo::qty::quantity::s_scaled_unit`` at runtime;
instead gets everything it needs at compile time.

Class
-----

The primary data structure for interacting with xo-unit is the
template class ``xo::qty::quantity``.
A quantity is a compile-time wrapper around a single arithmetic value,
with type taken from the ``Repr`` parameter in ``quantity<Unit, Repr>``.

.. doxygenclass:: xo::qty::quantity

Member Variables
----------------

.. doxygengroup:: quantity-static-vars
.. doxygengroup:: quantity-instance-vars

Type Traits
-----------

.. doxygengroup:: quantity-type-traits

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
