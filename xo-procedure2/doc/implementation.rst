.. _implementation:

.. toctree::
    :maxdepth: 2

Components
==========

Library dependency tower for *xo-procedure2*

.. ditaa::

    +--------------------------------+
    |              xo_gc             |
    +--------------------------------+
    |            xo_alloc2           |
    +--------------------------------+
    |            xo_facet            |
    +----------------+---------------+
    | xo_reflectutil | xo_indentlog  |
    +----------------+---------------+
    |            xo_cmake            |
    +--------------------------------+

Expect to have xo_facet depending on xo_arena instead of using std::unordered_map
