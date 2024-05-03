.. _install:

.. toctree
   :maxdepth: 2

Install
=======

`xo-unit source`_ lives on github.

.. _xo-unit source: https://github.com/rconybea/xo-unit

Implementation relies on some c++20 features (for example class-instances as template arguments).
Tested with gcc 12.3, 13.2

Include as submodule
--------------------

.. code-block:: bash

   cd myproject
   git submodule add -b main https://github.com/rconybea/xo-unit ext/xo-unit
   git submodule update --init

This assumes you organize directly-incorporated dependencies under directory ``myproject/ext``.
You would then add ``myproject/ext/xo-unit/include`` to your compiler's include path,
and add

.. code-block:: c++

   #include <xo/unit/quantity.hpp>

to c++ source files that rely on xo-unit

Supported compilers
-------------------

* developed with gcc 12.3.0 and gcc 13.2.0;  github CI using gcc 11.4.0 (asof March 2024)

Building from source
--------------------

Although the xo-unit library is header-only, unit tests have some dependencies.
Example instructions (github CI) for build starting from stock ubuntu are in `ubuntu-main.yml`_

.. _ubuntu-main.yml: https://github.com/Rconybea/xo-unit/blob/main/.github/workflows/ubuntu-main.yml

Unit test dependencies:

* `catch2`_ header-only unit-test framework
* `xo-cmake`_ cmake macros
* `xo-indentlog`_ logging with call-structure indenting
* `xo-refcnt`_ intrusive reference counting (needed by xo-reflect)
* `xo-subsys`_ plugin initialization support (needed by xo-reflect)
* `xo-reflect`_ c++ introspection library

.. _catch2: https://github.com/catchorg/Catch2
.. _xo-cmake: https://github.com/rconybea/xo-cmake
.. _xo-indentlog: https://github.com/rconybea/indentlog
.. _xo-refcnt: https://github.com/rconybea/refcnt
.. _xo-subsys: https://github.com/rconybea/subsys
.. _xo-reflect: https://github.com/rconybea/reflect
