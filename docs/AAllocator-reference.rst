.. _AAllocator-reference:

AAllocator Reference
====================

Abstract interface facet for arena allocator.
Provides simple arena allocation.

Context
-------

.. ditaa::
    :--scale: 0.99

    +--------------------------------+
    |      IAllocator_DArena         |
    +--------------------------------+
    |      IAllocator_Xfer           |
    +--------------------------------+
    |      IAllocator_ImplType       |
    +--------------+-----------------+
    |cBLU          |    DArena       |
    |   AAllocator +-----------------+
    |              | ArenaConfig     |
    +--------------+-----------------+

.. code-block:: cpp

    #incldue <xo/alloc2/AAllocator.hpp>

Class
-----

.. doxygenclass:: xo::mm::AAllocator

Methods
-------

.. doxygengroup:: mm-allocator-methods
