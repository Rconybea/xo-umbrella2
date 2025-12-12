.. _ArenaConfig-reference:

ArenaConfig Reference
=====================

Configuration for an arena allocator

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
    |              |    DArena       |
    |  AAllocator  +-----------------+
    |              | ArenaConfig cBLU|
    +--------------+-----------------+

.. code-block:: cpp

    #include <xo/alloc2/DArena.hpp>

Class
-----

.. doxygenclass:: xo::mm::ArenaConfig

.. doxygenclass:: mm-arenaconfig-instance-vars
