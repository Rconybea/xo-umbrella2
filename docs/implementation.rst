.. _implementation:

Components
==========

Library dependency tower for *xo-facet*:

.. ditaa::

    +-----------------+
    |    xo_facet     |
    +-----------------+
    |    xo_cmake     |
    +-----------------+

Abstraction tower for *xo-facet* components.

.. ditaa::
    :--scale: 0.85

    +--------------------------------+
    |             obj(A,D)           |
    +--------------------------------+
    |           RRouter(A,D)         |
    +--------------------------------+
    |           OObject(A,D)         |
    +--------------------------------+
    |        FacetImplType(A,D)      |
    +----------------+---------------+
    |     facet  [A] |     data  [D] |
    +----------------+---------------+

.. list-table:: Descriptions
   :header-rows: 1
   :widths: 18 30 50

   * - Component
     - Use
     - Description
   * - ``obj<A,D>``
     - ``x.foo()``
     - convenience wrapper with interface A, with state D*
   * - ``RRouter<A,D>``
     - ``x.foo()``
     - auto injects data pointer
   * - ``OObject<A,D>``
     - ``x.iface()->foo(x.data(), ..)``
     - fat object pointer. combine i/face + data pointer.
   * - ``FacetImplType<A,D>``
     - ``x.foo(void* data, ..)``
     - implement facet for a particular state datatype;
       explicit type-erased state
   * - facet
     - ``x.foo(void* data, ..)=0``
     - fully abstract interface; explicit type-erased state

.. uml::
    :caption: fat-object-pointer layout
    :scale: 99%
    :align: center

    object z1<<obj>>
    z1 : iface = vt1
    z1 : data = d1

    object vt1<<interface>>
    vt1 : foo()
    vt1 : bar()

    object d1<<data>>
    d1 : x = 0.6
    d1 : y = 0.8

    z1 o-- vt1
    z1 o-- d1
