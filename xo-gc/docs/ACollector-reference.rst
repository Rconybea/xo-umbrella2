.. _ACollector-reference:

ACollector Reference
====================

Abstract interface facet for generational garbage collector.

Context
-------

.. ditaa::
    :--scale: 0.99

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

.. code-block:: cpp

    #include <xo/gc/ACollector.hpp>

Class
-----

.. doxygenclass:: xo::mm::ACollector

Methods
-------

.. doxygengroup:: mm-collector-methods
