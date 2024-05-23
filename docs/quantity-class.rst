.. _quantity-class:

Quantity
========

Dimensioned quantity with compile-time unit checking/conversion

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+---------------+
    |cBLU quantity   |   xquantity   |
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

    object unit<<scaled_unit>>
    unit : is_natural() = true

    qty1 o-- unit : s_scaled_unit (static constexpr)

    }

-  Arithmetic on :doc:`xo::qty::quantity<quantity-reference>` instances
   does *not* use ``xo::qty::quantity::s_scaled_unit`` at runtime;
   instead gets everything it needs at compile time.

-  The :code:`xo::qty::quantity` template takes a :doc:`xo::qty::scaled_unit<scaled-unit-class>` instance,
   but only accepts values with :code:`xo::qty::scaled_unit::is_natural() == true`.

   This accomodation (instead of requiring a :doc:`xo::qty::natural_unit<natural-unit-class>` instance
   is to make possible code like this possible:

   .. code-block:: cpp

       #include "xo/unit/quantity.hpp"

       using namespace xo::qty;

       quantity<u::meter / u::second> x;
       quantity<u::meter * u::mter> y;

   while rejecting attempt to mix multiple scales in the same quantity value:

   .. code-block:: cpp

       quantity<u::meter * u::millimeter> x; // will not compile

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

Access Methods
--------------

.. doxygengroup:: quantity-access-methods

Constants
---------

.. doxygengroup:: quantity-constants

Conversion Methods
------------------

Amount-preserving conversion to quantities with different units and/or representation.

.. doxygengroup:: quantity-unit-conversion

Arithmetic Operators
--------------------

.. doxygengroup:: quantity-operators

Support methods for arithmetic operations

.. doxygengroup:: quantity-arithmetic-support

Comparison
----------

Support methods for comparison operators

.. doxygengroup:: quantity-comparison-support
