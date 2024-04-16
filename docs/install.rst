.. _install:

.. toctree
   :maxdepth: 2

Install
=======

`xo-flatstring source`_ lives on github.

.. _xo-flatstring source: https://github.com/rconybea/xo-flatstring

Implementation relies on c++20 features (for example class-instances as template arguments).
Tested with gcc 13.2

Include as submodule
--------------------

.. code-block:: bash

   cd myproject
   git submodule add -b main https://github.com/rconybea/xo-flatstring ext/xo-flatstring
   git submodule update --init

This assumes you organize directly-incorporated dependencies under directory ``myproject/ext``.
You would then add ``myproject/ext/xo-flatstring/include`` to your compiler's include path,
and from c++ do something like

.. code-block:: c++

   #include <xo/flatstring/flatstring.hpp>

in c++ source files that rely on xo-flatstring

Supported compilers
-------------------

* developed with gcc 13.2.0;  github CI using gcc 11.4.0 (asof April 2024)

Building from source
--------------------

Although the xo-flatstring library is header-only, unit tests have some dependencies.
Example instructions (github CI) for build starting from stock ubuntu are in `ubuntu-main.yml`_

.. _ubuntu-main.yml: https://github.com/Rconybea/xo-flatstring/blob/main/.github/workflows/ubuntu-main.yml

Unit test dependencies:

* `catch2`_ header-only unit-test framework
* `xo-cmake`_ cmake macros
* `xo-indentlog`_ logging with call-structure indenting

.. _catch2: https://github.com/catchorg/Catch2
.. _xo-cmake: https://github.com/rconybea/xo-cmake
.. _xo-indentlog: https://github.com/rconybea/indentlog
