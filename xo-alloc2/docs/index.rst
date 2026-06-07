# xo-alloc2 documentation master file

xo-alloc2 documentation
=======================

xo-alloc2 provides:

* Fast vm-aware arena allocation.
* Allocates uncommitted virtual memory, and commits on demand.
* When available, uses THP (Transparent Huge Pages) to mitigate pagetable pressure.
* Optional GC support, with per-alloc header.

Diagnostic features:

* with alloc headers: forward iterators over individual allocations
* configurable guard memory between allocations.

Implemented using FOMO (faceted rust-like object model) from xo-facet

.. toctree::
    :maxdepth: 2
    :caption: xo-alloc2 contents

    examples
    implementation
    AAllocator-reference
    IAllocator_Xfer-reference
    AAllocIterator-reference
    ArenaConfig-reference
    DArena-reference
    DArenaIterator-reference
    AllocInfo-reference
    cmpresult-reference
    glossary
    genindex
    search
