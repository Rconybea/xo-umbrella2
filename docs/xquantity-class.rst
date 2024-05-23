.. _xquantity-class:

Xquantity
=========

Polymorphic dimensioned quantity with runtime unit checking/conversion

Context
-------

.. ditaa::
    :--scale: 0.85

    +----------------+---------------+
    |    quantity    |cBLU xquantity |
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

    #include <xo/unit/xquantity.hpp>

.. uml::
    :scale: 99%
    :align: center

    allowmixing

    object qty1<<xquantity>>
    qty1 : scale = 1.23
    qty1 : unit = unit

    object unit<<scaled_unit>>
    unit : is_natural() = true

    qty1 o-- unit


Class
-----

Class with run-time unit representation.

.. doxygenclass:: xo::qty::xquantity

Member Variables
----------------

.. doxygengroup:: xquantity-instance-vars

Constructors
------------

.. doxygengroup:: xquantity-ctors

Access Methods
--------------

.. doxygengroup:: xquantity-access-methods

General Methods
---------------

.. doxygengroup:: xquantity-methods
