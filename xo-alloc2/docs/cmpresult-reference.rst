.. _cmpresult-reference:

cmpresult
=========

Represent the result of a partially ordered comparison

Context
-------

.. ditaa::
    :--scale: 0.99

    +----------------------+-------------------------+-----------------------------------+
    |        RAllocator    |   RAllocIterator        |       IAllocator_DArena           |
    |                      |                         |   IAllocIterator_DArenaIterator   |
    +----------------------+-------------------------+-----------------------------------+
    |    IAllocator_Xfer   |  IAllocIterator_Xfer    |              DArena               |
    |    IAllocator_Any    |  IAllocIterator_Any     |          DArenaIterator           |
    | IAllocator_Impltype  | IAllocIterator_Impltype |                                   |
    |                      |                         |                                   |
    +----------------------+-------------------------+-----------------------------------+
    |       AAllocator     |    AAllocIterator       |            ArenaConfig            |
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |              cBLU |
    +-----------------+----------------------------------------------+-------------------+

.. code-block:: cpp

    #include <xo/alloc2/cmpresult.hpp>

Class
-----

.. doxygenclass:: xo::mm::cmpresult

Constructors
------------

.. doxgyengroup:: mm-cmpresult-ctors

Methods
-------

.. doxygengroup:: mm-cmpresult-methods

Member Variables
----------------

.. doxygengroup:: mm-cmpresult-instance-vars
