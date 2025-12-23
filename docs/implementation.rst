.. _implementation:

Components
==========

Library dependency tower for *xo-alloc2*

.. ditaa::

    +-----------------+
    |    xo_alloc2    |
    +-----------------+
    |    xo_facet     |
    +-----------------+
    |    xo_cmake     |
    +-----------------+

Abstraction tower for *xo-alloc2* components

.. ditaa::
    :--scale: 0.99

    +--------------------------------+
    |      IAllocator_DArena         |
    +--------------------------------+
    |      IAllocator_Xfer           |
    |      IAllocator_Any            |
    +--------------+-----------------+
    |              |    DArena       |
    |  AAllocator  +-----------------+
    |              | ArenaConfig     |
    +--------------+-----------------+

.. list-table:: Descriptions
    :header-rows: 1
    :widths: 20 90

    * - Component
      - Description
    * - ``AAllocator``
      - allocator facet (abstract interface)
    * - ``DArena``
      - arena representation
    * - ``IAllocator_ImplType<D>``
      - lookup implementation for allocator A
        with representation D.
    * - ``IAllocator_Xfer<D>``
      - transfer interface. downcast to native state.
    * - ``IAllocator_DArena``
      - allocator implementation for ``DArena``
