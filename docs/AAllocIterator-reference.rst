.. _AAllocIterator-reference:

AAllocIterator Reference
========================

Abstract interface facet for an alloc iterator.

Base class for runtime polymorphism over alloc-iterator implementations,
using faceted object model.

* runtime size consists of vtable pointer only.

* per FOMO principles, runtime state is stored separately.
  Classes that inherit AAllocIterator will not add state.

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
    |       AAllocator     |    AAllocIterator   cBLU|            ArenaConfig            |
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |                   |
    +-----------------+----------------------------------------------+-------------------+

Application code will likely use:

.. code-block:: cpp

    #include <xo/alloc2/AllocIterator.hpp>

to get definitions for cooperating AllocIterator classes
:cpp:class:`xo::mm::AAllocIterator`,
:cpp:class:`xo::mm::IAllocIterator_Any`,
:cpp:class:`xo::mm::IAllocIterator_Xfer`,
:cpp:class:`xo::mm::RAllocator`

Instead, to get just :cpp:class:`xo::mm::AAllocIterator` definition:

.. code-block:: cpp

    #include <xo/alloc2/AAllocIterator.hpp>

Class
-----

.. doxygenclass:: xo::mm::AAllocIterator

Methods
-------

.. doxygengroup:: mm-allociterator-methods
