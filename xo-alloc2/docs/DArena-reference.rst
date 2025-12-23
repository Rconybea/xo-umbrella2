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


Arena memory layout
.. code-block:: text

      <----------------------------size-------------------------->
      <------------committed-----------><-------uncommitted------>
      <--allocated-->

      XXXXXXXXXXXXXXX___________________..........................
     ^               ^                  ^                         ^
     lo              free               limit                     hi

      [X] allocated:   in use
      [_] committed:   physical memory obtained
      [.] uncommitted: mapped in virtual memory, not backed by memory


Allocation layout
.. code-block:: text

                     free_(pre)
                         v

                         <-------------z1--------------->
                < guard ><  hz  ><     req_z     >< dz  >< guard >

      used <==  +++++++++0000zzzz@@@@@@@@@@@@@@@@@ppppppp+++++++++ ==> avail

                         ^       ^                                ^
                         header  mem                              |
                         ^                                        |
                         last_header_                   free_(post)

                 [+] guard after each allocation, for simple sanitize checks
                 [0] unused header bits (avail to application)
                 [z] record allocation size
                 [@] new allocated memory
                 [p] padding (to uintptr_t alignment)

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
