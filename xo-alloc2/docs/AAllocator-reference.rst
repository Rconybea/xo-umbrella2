.. _AAllocator-reference:

AAllocator Reference
====================

Abstract interface facet for an allocator.

Base class for runtime polymorphism over allocator implementations,
using faceted object model.

* runtime size consists of vtable pointer only.

* per FOMO prinicples, runtime state is stored separately.
  Classes that inherit ``AAllocator`` will not add state

Context
-------

.. ditaa::
    :--scale: 0.99

    +----------------------+-------------------------+-----------------------------------+
    |        RAllocator    |   RAllocIterator        |       IAllocator_DArena           |
    |                      |                         |   IAllocIterator_DArenaIterator   |
    +----------------------+-------------------------+-----------------------------------+
    |    IAllocator_Xfer   |  IAllocIterator_Xfer    |              DArena               |
    |    IAllocator_Any    |  IAllocIterator_Any     |          DArenaIterator           |
    | IAllocator_Impltype  | IAllocIterator_Impltype |                                   |
    |                      |                         |                                   |
    +----------------------+-------------------------+-----------------------------------+
    |cBLU   AAllocator     |    AAllocIterator       |            ArenaConfig            |
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |                   |
    +-----------------+----------------------------------------------+-------------------+

.. code-block:: cpp

    #include <xo/alloc2/AAllocator.hpp>

Class
-----

.. doxygenclass:: xo::mm::AAllocator

Types
-----

.. doxygengroup:: mm-allocator-type-traits

Methods
-------

.. doxygengroup:: mm-allocator-methods
