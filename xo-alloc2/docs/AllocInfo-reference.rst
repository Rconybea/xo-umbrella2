.. _AllocInfo-reference:

AllocInfo Reference
===================

Information, including alloc metadata, pertaining to a particular allocation.

Context
-------

.. ditaa::
    :--scale: 0.99

    +----------------------+-------------------------+-----------------------------------+
    |        RAllocator    |   RAllocIterator        |       IAllocator_DArena           |
    |                      |                         |   IAllocIterator_DArenaIterator   |
    +----------------------+-------------------------+-----------------------------------+
    |    IAllocator_Xfer   |   IAllocIterator_Xfer   |              DArena               |
    |    IAllocator_Any    |   IAllocIterator_Any    |          DArenaIterator           |
    | IAllocator_Impltype  | IAllocIterator_Impltype |                                   |
    |                      |                         |                                   |
    +----------------------+-------------------------+-----------------------------------+
    |       AAllocator     |    AAllocIterator       |            ArenaConfig       cBLU |
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |                   |
    +-----------------+----------------------------------------------+-------------------+

.. code-block:: cpp

    #include <xo/alloc2/DArena.hpp>


Class
-----

.. doxygenclass:: xo::mm::AllocInfo

Member Variables
----------------

.. doxygengroup:: mm-allocinfo-instance-vars

Type Traits
-----------

.. doxygengroup:: mm-allocinfo-traits

Constructors
------------

.. doxygengroup:: mm-allocinfo-ctors

Methods
-------

.. doxygengroup:: mm-allocinfo-methods
