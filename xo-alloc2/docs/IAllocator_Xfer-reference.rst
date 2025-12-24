.. _IAllocator_Xfer-reference:

IAllocator_Xfer
===============

IAllocator_Xfer provides a type-erased interface to a specific native allocator
implementation.

Context
-------

.. ditaa::
    :--scale: 0.99

    +----------------------+-------------------------+-----------------------------------+
    |        RAllocator    |   RAllocIterator        |       IAllocator_DArena           |
    |                      |                         |   IAllocIterator_DArenaIterator   |
    +----------------------+-------------------------+-----------------------------------+
    |cBLU IAllocator_Xfer  |  IAllocIterator_Xfer    |              DArena               |
    +----------------------+  IAllocIterator_Any     |          DArenaIterator           |
    |    IAllocator_Any    | IAllocIterator_Impltype |                                   |
    | IAllocator_Impltype  |                         |                                   |
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

Each method operates by downcasting its first argument to the known target
native interface type, and delegating to native interface method with the same name.

For example (paraphrasing):

.. code-block:: cpp

    template <typename DRepr, typename IAllocator_DRepr>
    bool IAllocator_Xfer<DRepr, IAllocator_DRepr>::contains(Copaque d,
                                                            const void * p) const ...
    {
      return IAllocator_DRepr::contains(*(DRepr*)(d), p);
    };

Code for other methods follows the same pattern;
in fact expect to be able to generate class definition mechanically at some point.

Application code will not normally interact with :cpp:class:`IAllocator_Xfer`.
It will be used once for each specific allocator implementation
(e.g. with :cpp:class:`xo::mm::IAllocator_DArena`).
In any case, can include the transfer template with:

.. code-block:: cpp

    #include <xo/alloc2/IAllocator_Xfer.hpp>

Class
-----

.. doxygenclass:: xo::mm::IAllocator_Xfer

Types
-----

.. doxygengroup:: mm-allocator-xfer-types

Methods
-------

.. doxygengroup:: mm-allocator-xfer-methods
