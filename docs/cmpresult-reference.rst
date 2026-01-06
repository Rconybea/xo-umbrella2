.. _cmpresult-reference:

cmpresult
=========

Represent the result of a partially ordered comparison

Context
-------

.. ditaa::
    :--scale: 0.99

    +-----------------------------------------------------+
    |                       DArena                        |
    |                   DArenaIterator                    |
    +-----------------------------------------------------+
    |                     ArenaConfig                     |
    +--------------+------------------------+-------------+
    |              |        AllocInfo       |         cBLU|
    |              +------------------------+             |
    | AllocError   |  AllocHeaderConfig     |  cmpresult  |
    |              +------------------------+             |
    |              |      AllocHeader       |             |
    +--------------+------------------------+-------------+


.. code-block:: cpp

    #include <xo/arena/cmpresult.hpp>

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
