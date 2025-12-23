.. _AAllocator-reference:

AAllocator Reference
====================

Abstract interface facet for arena allocator.
Provides simple arena allocation.

Context
-------

.. ditaa::
    :--scale: 0.99

    +-------------------------------+--------------------------------------+
    |                               |  IAllocator_DX1Collector             |
    |                               |  IAllocIterator_DX1CollectorIterator |
    | IAllocator_DArena             |                                      |
    | IAllocIterator_DArenaIterator +--------------------------------------+
    |                               |             DX1Collector             |
    |                               |        DX1CollectorIterator          |
    |                               |                                      |
    +-------------------------------+---------+----------------------------+
    |               DArena                    |                            |
    |               DArenaIterator            |                            |
    +-----------------------------------------+       CollectorConfig      |
    |               ArenaConfig               |                            |
    +-----------------------------------------+----------------------------+

    +----------------------------------+-----------------------------------+
    |            RAllocator            |           RAllocIterator          |
    +----------------------------------+-----------------------------------+
    |        IAllocator_DX1Collector   |     IAllocIterator_DX1Collector   |
    |        IAllocator_DArena         |     IAllocIterator_DArena         |
    +----------------------------------+-----------------------------------+
    |        IAllocator_Xfer           |     IAllocIterator_Xfer           |
    |        IAllocator_Any            |     IAllocIterator_Any            |
    +----------------------------------+-----------------------------------+
    |           AAllocator             |           AAllocIterator          |
    +----------------------------------+-----------------------------------+

    +---------------+------------------+----------------------+------------+
    |               |                  |       AllocInfo      |            |
    |  generation   |                  +----------------------+            |
    |  object_age   |    AllocError    |   AllocHeaderConfig  |  cmpresult |
    |     role      |                  +----------------------+            |
    |               |                  |     AllocHeader      |            |
    +---------------+------------------+----------------------+------------+

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
