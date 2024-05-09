.. _bpu-class:

BPU
===

.. code-block:: cpp

    #include <xo/unit/bpu.hpp>

A :code:`xo::qty::bpu` represents a rational (usually integer) power of a :doc:`basis-unit-class`.

For example:

.. uml::
    :caption: representation for cubic millimeters
    :scale: 99%
    :align: center

    object mm3<<bpu>>
    mm3 : native_dim = dim::distance
    mm3 : scalefactor = 1/1000
    mm3 : power = 3/1

.. uml::
    :caption: representation for annual (365-day) volatility
    :scale: 99%
    :align: center

    object vol<<bpu>>
    vol : native_dim = dim::time
    vol : scalefactor = 365*24*3600
    vol : power = -1/2
