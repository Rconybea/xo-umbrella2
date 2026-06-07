.. _implementation:

Implementation
==============

Library dependency tower for *xo-arena*

.. ditaa::

    +------------------------------------+
    |             xo_arena               |
    +-----------------+------------------+
    |   xo_indentlog  |  xo_reflectutil  |
    +-----------------+------------------+
    |             xo_cmake               |
    +------------------------------------+

Abstraction tower for *xo-arena* components (simplified)

.. ditaa::
    :--scale: 0.99

    +-------------------+
    |   DArena          |
    |   DArenaIterator  |
    +-------------------+
    |   ArenaConfig     |
    +-------------------+
    |  auxiliary types  |
    +-------------------+


Abstraction tower for *xo-arena* components (detailed)

.. ditaa::
    :--scale: 0.99

    +-----------------------------------------------------+
    |                       DArena                        |
    |                   DArenaIterator                    |
    +-----------------------------------------------------+
    |                     ArenaConfig                     |
    +--------------+------------------------+-------------+
    |              |        AllocInfo       |             |
    |              +------------------------+             |
    | AllocError   |  AllocHeaderConfig     |  cmpresult  |
    |              +------------------------+             |
    |              |      AllocHeader       |             |
    +--------------+------------------------+-------------+

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

    object darena1<<DArena>>
    darena1 : config
    darena1 : lo
    darena1 : hi
    darena1 : free
    darena1 : limit
    darena1 : last_error

    object header1<<ArenaConfig>>
    header1 : size
    header1 : header

    object hconfig1<<AllocHeaderConfig>>
    hconfig1 : guard_z
    hconfig1 : guard_byte
    hconfig1 : tseq_bits
    hconfig1 : age_bits
    hconfig1 : size_bits

    darena1 o-- header1
    header1 o-- hconfig1

.. uml::
    :caption: memory layout
    :scale: 99%
    :align: center

    object darena1<<DArena>>
    darena1 : config
    darena1 : lo
    darena1 : hi
    darena1 : free
    darena1 : limit
    darena1 : last_error

    rectangle "allocated" #90EE90 {
      note as n1
        lo -> free
        objects here
      end note
    }

    rectangle "available" #FFFFE0 {
      note as n2
        free -> limit
        alloc from here
      end note
    }

    rectangle "uncommitted" #D3D3D3 {
      note as n3
        limit -> hi
        not mapped yet
      end note
    }

    darena1 -[hidden]down- n1
    n1 -[hidden]down- n2
    n2 -[hidden]down- n3

Remarks:

* See xo-alloc2 for abstract allocator trait *AAllocator*
  along with its application to *DArena*.
* We split these because in *xo-facet* we rely on *DArena* to implement
  double-dispatch (two-dimensional vtables, as seen for example in CLOS, Julia, Mathematica).
