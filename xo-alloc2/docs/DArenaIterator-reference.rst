.. _DArenaIterator-reference:

DArenaIterator
==============

Iterator for allocs obtained from a :cpp:class:`DArena`.

Context
-------

.. ditaa::
    :--scale: 0.99

    +----------------------+-------------------------+-----------------------------------+
    |        RAllocator    |   RAllocIterator        |       IAllocator_DArena           |
    |                      |                         |   IAllocIterator_DArenaIterator   |
    +----------------------+-------------------------+-----------------------------------+
    |    IAllocator_Xfer   |   IAllocIterator_Xfer   |              DArena               |
    |    IAllocator_Any    |   IAllocIterator_Any    +-----------------------------------+
    | IAllocator_Impltype  | IAllocIterator_Impltype |          DArenaIterator           |
    |                      |                         |                               cBLU|
    +----------------------+-------------------------+-----------------------------------+
    |       AAllocator     |    AAllocIterator       |            ArenaConfig            |
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |                   |
    +-----------------+----------------------------------------------+-------------------+

.. code-block:: cpp

    #include <xo/alloc2/DArenaIterator.hpp>

Class
-----

.. doxygenclass:: xo::mm::DArenaIterator

Member Variables
----------------

.. doxygengroup:: mm-arenaiterator-instance-vars

Constructors
------------

.. doxygengroup:: mm-arenaiterator-ctors

Methods
-------

.. doxygengroup:: mm-arenaiterator-methods
