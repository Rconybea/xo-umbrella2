.. _DArena-reference:

DArena Reference
================

Native representation for arena allocator

Context
-------

.. ditaa::
    :--scale: 0.99

    +--------------------------------+
    |      IAllocator_DArena         |
    +--------------------------------+
    |      IAllocator_Xfer           |
    +--------------------------------+
    |      IAllocator_ImplType       |
    +--------------+-----------------+
    |              |    DArena   cBLU|
    |   AAllocator +-----------------+
    |              | ArenaConfig     |
    +--------------+-----------------+

.. code-block:: cpp

    #include <xo/alloc2/DArena.hpp>


Arena memory layout::

    <----------------------------size-------------------------->
    <------------committed-----------><-------uncommitted------>
    <--allocated-->

    XXXXXXXXXXXXXXX___________________..........................
   ^               ^                  ^                         ^
   lo              free               limit                     hi

    [X] allocated:   in use
    [_] committed:   physical memory obtained
    [.] uncommitted: mapped in virtual memory, not backed by memory


Class
-----

.. doxygenclass:: xo::mm::DArena

Member Variables
----------------

.. doxygengroup:: mm-arena-instance-vars

Type Traits
-----------

.. doxygengroup:: mm-arena-traits

Constructors
------------

.. doxygengroup:: mm-arena-ctors
