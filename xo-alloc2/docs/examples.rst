.. _examples:

.. toctree
    :maxdepth: 2

Examples
========

Arena allocation
-----------------

.. code-block:: cpp

    #include <xo/alloc2/arena/DArena.hpp>

    using namespace xo::mm;
    using namespace std;


Create an arena:

.. code-block:: cpp

    // create arena, size 64k
    DArena arena = DArena::map(ArenaConfig { .size_ = 64*1024; });

    cout << arena.lo() << ".." << arena.hi();

This determines a VM memory address range.
Actually address range is rounded up to a whole number of VM pages.
Size here is a hard maximum. It cannot be changed for this arena instance.

.. code-block:: cpp

    arena.reserved(); // 64k
    arena.committed(); // 0k
    arena.available(); // 0k

Although we know the address range for arena, it doesn't own any physical
memory yet.
