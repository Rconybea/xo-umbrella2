.. _DArena-reference:

DArena
======

Native arena allocator

Context
-------

.. ditaa::
    :--scale: 0.99

    +-----------------------------------------------------+
    |                       DArena                    cBLU|
    |                   DArenaIterator                    |
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

    #include <xo/arena/DArena.hpp>

Arena memory layout
~~~~~~~~~~~~~~~~~~~

.. code-block:: text

      <------------------------reserved-------------------------->
      <------------committed-----------><-------uncommitted------>
      <--allocated--><----available---->

      XXXXXXXXXXXXXXX___________________..........................
     ^               ^                  ^                         ^
     lo              free               limit                     hi

      [X] allocated:   in use
      [_] committed:   physical memory obtained
      [.] uncommitted: mapped in virtual memory, not backed by memory


Representation for a single allocation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

                 [+] guard surrounding each allocation, for simple sanitize checks
                 [0] unused header bits (available for application metadata)
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

Methods
-------

.. doxygengroup:: mm-arena-methods
