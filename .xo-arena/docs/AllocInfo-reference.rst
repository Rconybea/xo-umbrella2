.. _AllocInfo-reference:

AllocInfo Reference
===================

Describes a single allocation.
Requires allocator configured to store per-allocation headers

Context
-------

.. ditaa::
    :--scale: 0.99

    +-----------------------------------------------------+
    |                       DArena                        |
    |                   DArenaIterator                    |
    +-----------------------------------------------------+
    |                     ArenaConfig                     |
    +--------------+------------------------+-------------+
    |              |        AllocInfo   cBLU|             |
    |              +------------------------+             |
    | AllocError   |  AllocHeaderConfig     |  cmpresult  |
    |              +------------------------+             |
    |              |      AllocHeader       |             |
    +--------------+------------------------+-------------+
