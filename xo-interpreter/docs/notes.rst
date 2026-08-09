.. _notes

.. toctree
   :maxdepth: 2

Notes
=====

Currently (Nov 2025) using reference-counted pointers for `xo::scm::Expression`, and
garbage-collected / arena-allocated pointers for `xo::scm::Env`.
Maybe want to refactor parser to also use garbage-collected (gc) pointers for `xo::scm::Expression`.
That said, Should be possible to keep reference-counted Expressions indefinitely, given they
cannot contain cycles.

For interpreted behavior, expect gc pointers to be faster
