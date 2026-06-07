.. _ArenaConfig-reference:

ArenaConfig Reference
=====================

Configuration for an arena allocator

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
    |       AAllocator     |    AAllocIterator       |            ArenaConfig        cBLU|
    +----------------------+-------------------------+-----------------------------------+
    +-----------------+----------------------------------------------+-------------------+
    |                 |                    AllocInfo                 |                   |
    |                 +----------------------------------------------+                   |
    |    AllocError   |                AllocHeaderConfig             |     cmpresult     |
    |                 +----------------------------------------------+                   |
    |                 |                  AllocHeader                 |                   |
    +-----------------+----------------------------------------------+-------------------+

.. uml::
    :caption: example arena config
    :scale: 99%
    :align: center

    object cfg<<AreanConfig>>
    cfg : name = "tmp"
    cfg : size = 128MB
    cfg : hugepage_z = 2MB
    cfg : guard_z = 8
    cfg : guard_byte = 0xfd
    cfg : store_header_flag = true
    cfg : header_size_mask = 0xffffffff
    cfg : debug_flag = false

.. code-block:: cpp

    #include <xo/alloc2/ArenaConfig.hpp>

Class
-----

.. doxygenclass:: xo::mm::ArenaConfig

Instance Variables
------------------

.. doxygengroup:: mm-arenaconfig-instance-vars
