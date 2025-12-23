# xo-alloc2 documentation master file

xo-alloc2 documentation
=======================

xo-alloc2 is intended to provide fast vm-aware arena allocation.
Next-generation version of xo-alloc.

Features:

* allocates uncommitted virtual memory, and commits on demand.
* uses THP (Transparent Huge Pages) when available.

Implemented using FOMO (faceted rust-like object model) from xo-facet

.. toctree::
    :maxdepth: 2
    :caption: xo-alloc2 contents

    implementation
    AAllocator-reference
    ArenaConfig-reference
    DArena-reference
    glossary
    genindex
    search
