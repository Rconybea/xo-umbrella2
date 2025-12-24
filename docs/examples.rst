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

    arena.reserved();  // 64k
    arena.committed(); // 0k
    arena.allocated(); // ok
    arena.available(); // 0k

Although we know the address range for arena, it doesn't own any physical
memory yet.  Two ways to commit memory:

1. Attempt allocation:

   .. code-block:: cpp

       std::byte * mem = arena.alloc(5*1024);
       if (!mem)
           throw std::runtime_error("alloc failed");

       arena.reserved();   // 64k
       arena.committed();  // 8k - 2 pages
       arena.allocateed(); // 5k
       arena.available();  // 3k

2. Expand committed memory explicitly:

   .. code-block:: cpp

       bool ok = arena.expand(5*1024);
       assert(ok);

       arena.reserved();  // 64k
       arena.committed(); // 8k - 2 pages
       arena.allocated(); // 0k
       arena.available(); // 8k

Examining alloc metadata
------------------------

Given a successful allocation:

.. code-block:: cpp

    std::size_t req_z = 5*1024;
    std::byte * mem = arena.alloc(req_z);
    if (!mem)
        throw std::runtime_error("alloc failed");

    AllocInfo info = arena.alloc_info(mem);

    info.payload();  // [mem, mem + req_z (+ up to 7 bytes padding)]
    info.is_valid(); // true
    info.guard_lo(); // guard bytes preceding alloc
    info.guard_hi(); // guard bytes following alloc

Recycling memory
----------------

.. code-block:: cpp

    // arena in non-empty state
    arena.reserved();   // 64k
    arena.committed();  // 8k - 2 pages
    arena.allocateed(); // 5k
    arena.available();  // 3k

    arena.clear();

    arena.reserved()    // 64k
    arena.committed();  // 8k - 2 pages
    arena.allocated();  // 0k
    arena.available();  // 8k

Memory released by @ref Darena::clear is still committed.
It's in use as far as operating system is concerned.

To release memory to the operating system, destroy arena:

.. code-block:: cpp

    arena.~DArena();  // or just let arena go out of scope
