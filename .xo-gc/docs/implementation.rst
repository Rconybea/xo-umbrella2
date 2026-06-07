.. _implementation:

.. toctree::
    :maxdepth: 2

Components
==========

Library dependency tower for *xo-gc*

.. ditaa::


    +----------------+
    |     xo_gc      |
    +----------------+
    |   xo_alloc2    |
    +----------------+
    |    xo_facet    |
    +----------------+
    |    xo_cmake    |
    +----------------+

Abstraction tower for *xo-gc* components:

.. ditaa::
    :--scale: 0.85

    +--------------------------------------------------+-----------------+
    |  IAllocIterator_DX1CollectorIterator             |                 |
    |  IAllocator_DX1Collector                         |  RGCObject      |
    |  ICollector_DX1Collector                         |  IGCObject_Xfer |
    |  ICollector_Xfer                                 |  IGCObject_Any  |
    |  ICollector_Any                                  |                 |
    +--------------------------------------------------+-----------------+
    +----------------------+--------------+------------+-----------------+
    | DX1CollectorIterator | DX1Collector | ACollector |  AGCObject      |
    |                      |              |            |                 |
    +----------------------+--------------+------------+-----------------+
    +--------------------------------------------------------------------+
    |    CollectorConfig      generation      object_age     role        |
    +--------------------------------------------------------------------+
