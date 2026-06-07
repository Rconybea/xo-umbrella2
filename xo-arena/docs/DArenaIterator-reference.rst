.. _DArenaIterator-reference:

DArenaIterator
==============

Iterator for allocs obtained from a :cpp:class:`xo::mm::DArena`.

Context
-------

.. ditaa::
    :--scale: 0.99

    +-----------------------------------------------------+
    |                       DArena                        |
    |                   DArenaIterator                cBLU|
    +-----------------------------------------------------+
    |                     ArenaConfig                     |
    +--------------+------------------------+-------------+
    |              |        AllocInfo       |             |
    |              +------------------------+             |
    | AllocError   |  AllocHeaderConfig     |  cmpresult  |
    |              +------------------------+             |
    |              |      AllocHeader       |             |
    +--------------+------------------------+-------------+


.. code-block:: cpp

    #include <xo/arena/DArenaIterator.hpp>

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
