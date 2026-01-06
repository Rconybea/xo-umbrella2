# xo-arena documentation master file

xo-arena documentation
======================

xo-arena provides:

* Fast vm-aware arena allocation.
* Allocates uncommitted virtual memory, and commits on demand.
* When available, uses THP (Transparent Huge Pages) to mitigate pagetable pressure.
* Optional GC support, with per-alloc header.

Diagnostic features:

* with alloc headers: forward iterators over individual allocations
* configurable guard memory between allocations.

.. toctree::
    :maxdepth: 2
    :caption: xo-arena contents

    examples
    implementation
    ArenaConfig-reference
    DArena-reference
    DArenaIterator-reference
    AllocInfo-reference
    cmpresult-reference
    glossary
    genindex
    search
