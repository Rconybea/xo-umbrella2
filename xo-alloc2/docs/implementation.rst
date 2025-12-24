.. _implementation:

Implementation
==============

Library dependency tower for *xo-alloc2*

.. ditaa::

    +-----------------+
    |    xo_alloc2    |
    +-----------------+
    |    xo_facet     |
    +-----------------+
    |    xo_cmake     |
    +-----------------+

Abstraction tower for *xo-alloc2* components (simplified)

.. ditaa::
    :--scale: 0.99

    +----------------+-----------------+-------------------+
    |                |                 |   DArena          |
    |  Allocator     |  AllocIterator  |   DArenaIterator  |
    |                |                 +-------------------+
    |                |                 |   ArenaConfig     |
    +----------------+-----------------+-------------------+
    |                     auxiliary types                  |
    +------------------------------------------------------+


Abstraction tower for *xo-alloc2* components (detailed)

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
    |       AAllocator     |    AAllocIterator       |            ArenaConfig            |
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |                   |
    +-----------------+----------------------------------------------+-------------------+

.. list-table:: Polymorphic Allocator
    :header-rows: 1
    :widths: 20 90

    * - Class
      - Description
    * - ``AAllocator``
      - Abstract allocator interface for runtime polymorphism
    * - ``IAllocator_Any``
      - Stub allocator interface for uninitialized variant
    * - ``IAllocator_Xfer<D>``
      - Allocator interface template for representation ``D``
    * - ``IAllocator_Impltype<D>``
      - Lookup allocator interface for representation ``D``
    * - ``RAllocator<O>``
      - Provide allocator methods for FOMO object ``O``

.. list-table:: Polymorphich Alloc Iterator
    :header-rows: 1
    :widths: 20 90

    * - Class
      - Description
    * - ``AAllocIterator``
      - Abstract interface for iteration over allocs
    * - ``IAllocIterator_Any``
      - Stub alloc-iterator interface for uninitialized variant
    * - ``IAllocIterator_Xfer<D>``
      - Alloc-iterator interface template for representation ``D``
    * - ``IAllocIterator_Impltype<D>``
      - Lookup alloc-iterator interface for representation ``D``
    * - ``RAllocIterator<D>``
      - Provide alloc-iterator methods for FOMO object ``O``.

.. list-table:: Native Arena Allocator
    :header-rows: 1
    :widths: 20 90

    * - Class
      - Description
    * - ``ArenaConfig``
      - Configuration for a ``DArena`` instance
    * - ``DArena``
      - VM-aware arena allocator
    * - ``DArenaIterator``
      - Iterator over ``DArena`` allocations
    * - ``IAlllocator_DArena``
      - Adapt a ``DArena`` to facet ``AAllocator``
    * - ``IAllocIterator_DArenaAllocator``
      - Adapt a ``DArenaIterator`` to facet ``AAllocIterator``

.. list-table:: Auxiliary/Support Types
    :header-rows: 1
    :widths: 20 90

    * - Class
      - Description
    * - ``AllocError``
      - Return type for an alloc request, with error details.
    * - ``AllocInfo``
      - An opaque allocation. Value of an alloc-iterator.
    * - ``AllocHeaderConfig``
      - Per-allocator configuration of alloc headers
    * - ``AllocHeader``
      - Per-allocation header (8 bytes)
    * - ``cmpresult``
      - Result of alloc-iterator comparison

Example Object Diagram

.. uml::
    :caption: representation for an arena allocator
    :scale: 99%
    :align: center

    object rarena1<<RAllocator>>
    rarena1 : iface = vtable1
    rarena1 : data = darena1

    object vtable1<<IAllocator_DArena_vtable>>
    vtable1 : alloc()

    object darena1<<DArena>>
    darena1 : config
    darena1 : lo
    darena1 : hi
    darena1 : free
    darena1 : limit
    darena1 : last_error

    rarena1 o-- vtable1
    rarena1 o-- darena1


Remarks:

* When we know the allocator representation at compile time (``DArena`` here),
  then we also know the interface (``IAllocator_DArena``).
  Devirtualization is easy since interface methods are all final.

* Size of a FOMO object is two pointers; it's natural to create such objects
  on the fly and pass them by value.
  When storing an allocator in another data structure, we only need to use
  the RAllocator stack if we want runtime polymorphism for the stored allocator.
  Otherwise can store a ``DArena`` instance.
