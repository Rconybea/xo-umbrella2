
.. _span-class:

Span
====

Identify an unowned contiguous memory range

Context
-------

.. ditaa::
    :--scale: 0.85

    +-----------------------------------------+----------+
    |                tokenizer                |          |
    +-----------------------------------------+          |
    |               scan_result               |          |
    +-----------------+-----------------------+          |
    |                 |     tokenizer_error   |  buffer  |
    |     token       +-----------------------+          |
    |                 |      input_state      |          |
    +-----------------+-----------------------+          |
    |     tokentype   |cBLU      span         |          |
    +-----------------+-----------------------+----------+

.. code-block:: cpp

    #include <xo/tokenizer/span.hpp>

.. uml::
    :scale: 99%
    :align: center

    allowmixing

    object span1<<span>>
    span1 : lo = p
    span1 : hi = p+25

    object dest<<memory>>
    dest : def fact(n : i64) { ... }

    span1 o-- dest

- Identify a sequence of characters stored in contiguous memory.

- Lightweight, consists of a pair of pointers.

- Does not own storage. Lifetime management for target memory is
  up to the caller.


Class
-----

.. doxygenclass:: xo::scm::span

Member Variables
----------------

.. doxygengroup:: span-instance-vars

Type Traits
-----------

.. doxygengroup:: span-type-traits

Constructors
------------

.. doxygengroup:: span-ctors

Access Methods
--------------

.. doxygengroup:: span-access-methods

General Methods
---------------

.. doxygengroup:: span-general-methods

Operators
---------

.. doxygengroup:: span-operators
