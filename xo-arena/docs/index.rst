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

.. figure:: _static/img/arena-snapshot.svg
    :alt: Arena showing allocated, available and uncommitted regions.
    :align: center
    :width: 100%
    :target: lifecycle.html

    An arena after some allocations.  ``lo_`` to ``free_`` is allocated -- payload plus
    per-allocation overhead; ``free_`` to ``limit_`` is available, committed but not yet
    handed out.  Together they are the committed memory, which always ends on a superpage
    boundary.  Past ``limit_`` the range is uncommitted: addresses and nothing more.
    See :doc:`lifecycle` to watch it move.

.. toctree::
    :maxdepth: 2
    :caption: xo-arena contents

    examples
    lifecycle
    implementation
    ArenaConfig-reference
    DArena-reference
    DArenaIterator-reference
    AllocInfo-reference
    cmpresult-reference
    glossary
    genindex
    search
