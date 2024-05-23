.. _install:

.. toctree
   :maxdepth: 2

Install
=======

``xo-unit`` uses supporting header-only libraries ``xo-ratio`` and ``xo-flatstring``.
and (optionally) cmake macros ``xo-cmake``.  These are on github:

- `xo-unit source`_ (constexpr quantities, units and dimension-checking)
- `xo-ratio source`_ (constexpr exact ratios)
- `xo-flatstring source`_ (constexpr strings)
- `xo-cmake source`_ (shared cmake macros)

.. _xo-unit source: https://github.com/rconybea/xo-unit
.. _xo-ratio source: https://github.com/rconybea/xo-ratio
.. _xo-flatstring source: https://github.com/rconybea/xo-flatstring
.. _xo-cmake source: https://github.com/rconybea/xo-cmake

`xo-cmake` is nccessary to invoke `xo` cmake build for:

-  site install
-  example programs
-  unit tests

Can omit to instead copy `xo_unit`, `xo-ratio` and `xo-flatstring` source trees;
see instructions below for including as git submodule

Implementation relies on some c++20 features (for example class-instances as template arguments).
Tested with gcc 12.3, 13.2

Include as submodule
--------------------

One way to use ``xo-unit`` in a project is to import the source tree directly:

.. code-block:: bash

   cd myproject
   git submodule add -b main https://github.com/rconybea/xo-flatstring ext/xo-flatstring
   git submodule add -b main https://github.com/rconybea/xo-ratio ext/xo-ratio
   git submodule add -b main https://github.com/rconybea/xo-unit ext/xo-unit
   git submodule update --init

This assumes you organize directly-incorporated dependencies under directory ``myproject/ext``.
You would then add to your compiler's include path the directories ``myproject/ext/xo-flatstring/include``,
``myproject/ext/xo-ratio/include``, ``myproject/ext/xo-unit/include``;

and add

.. code-block:: c++

   #include <xo/unit/quantity.hpp>

to c++ source files that rely on xo-unit

Ubuntu Install Pattern
----------------------

:doc:`Example instructions<ubuntu-github-workflow>` (used for github actions CI) for build starting from stock ubuntu

Installing from source
----------------------

Install scripts for `xo-unit`, `xo-ratio` and `xo-flatstring` depend on shared cmake macros
and a bootstrap script `xo-cmake-config`.

* `xo-cmake`_ source

.. _xo-cmake: https://github.com/rconybea/xo-cmake

Preamble:

.. code-block:: bash

   mkdir -p ~/proj/xo
   cd ~/proj/xo

   git clone https://github.com/rconybea/xo-cmake
   git clone https://github.com/rconybea/xo-flatstring
   git clone https://github.com/rconybea/xo-ratio
   git clone https://github.com/rconybea/xo-unit

   PREFIX=~/local  # ..or other desired installation prefix

Ordering below is important;  cmake support for each subsystem
requires successful installation of its predecessor.

Install `xo-cmake`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-cmake/.build -S xo-cmake
    cmake --build xo-cmake/.build -j   # placeholder, no-op for now
    cmake --install xo-cmake/.build

Build and Install `xo-flatstring`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DENABLE_TESTING=0 \
          -B xo-flatstring/.build -S xo-flatstring
    cmake --build xo-flatstring/.build -j
    cmake --install xo-flatstring/.build

Build and Install `xo-ratio`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DENABLE_TESTING=0 \
          -B xo-ratio/.build -S xo-ratio
    cmake --build xo-ratio/.build -j
    cmake --install xo-ratio/.build

Build and Install `xo-unit`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DENABLE_TESTING=0 \
          -B xo-unit/.build -S xo-unit
    cmake --build xo-unit/.build -j
    cmake --install xo-unit/.build

Directories under ``PREFIX`` will then contain:

.. code-block::

    PREFIX
    +- bin
    |  \- xo-cmake-config
    +- include
    |  \- xo
    |     +- cxxutil/
    |     +- flatstring/
    |     +- ratio/
    |     +- unit/
    +- lib
    |  \- cmake
    |     +- indentlog/
    |     +- randomgen/
    |     +- xo_flatstring/
    |     \- xo_unit/
    +- share
       \- cmake
          \- xo_macros/

Use CMake Support
-----------------

To use built-in cmake suport:

Make sure ``PREFIX/lib/cmake`` is searched by cmake (if necessary, include it in ``CMAKE_PREFIX_PATH``)

Add to ``CMakeLists.txt``:

.. code-block:: cmake

    FindPackage(xo_unit CONFIG REQUIRED)

    target_link_libraries(mytarget INTERFACE xo_unit)

Build and Install with Unit Tests Enabled
-----------------------------------------

Running unit tests require a few additional dependencies:

* `catch2`_ header-only unit-test framework
* `xo-indentlog`_ logging with call-structure indenting
* `xo-randomgen`_ fast random number generator (xoshiro256ss)

.. _catch2: https://github.com/catchorg/Catch2
.. _xo-indentlog: https://github.com/rconybea/indentlog
.. _xo-randomgen: https://github.com/rconybea/randomgen

Preamble:

.. code-block:: bash

   mkdir -p ~/proj/xo
   cd ~/proj/xo

   git clone https://github.com/rconybea/xo-cmake
   git clone https://github.com/rconybea/indentlog xo-indentlog
   git clone https://github.com/rconybea/randomgen xo-randomgen
   git clone https://github.com/rconybea/xo-flatstring
   git clone https://github.com/rconybea/xo-ratio
   git clone https://github.com/rconybea/xo-unit

   PREFIX=~/local  # ..or other desired installation prefix

Build and Install `catch2` (assuming ubuntu here):

.. code-block:: bash

    sudo apt-get install catch2  # on ubuntu, for example

Build and Install `xo-cmake`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-cmake/.build -S xo-cmake
    cmake --build xo-cmake/.build -j   # placeholder, no-op for now
    cmake --install xo-cmake/.build

Build, Test and Install `xo-indentlog`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-indentlog/.build -S xo-indentlog
    cmake --build xo-indentlog/.build -j
    cmake --build xo-indentlog/.build -- test   # run unit tests, cmake invokes ctest
    (cd xo-indentlog/.build && ctest)           # or invoke ctest directly
    cmake --install xo-indentlog/.build

Build and Install `xo-randomgen` (no unit tests yet):

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-randomgen/.build -S xo-randomgen
    cmake --build xo-randomgen/.build -j
    cmake --install xo-randomgen/.build

Build, Test and Install `xo-flatstring`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-flatstring/.build -S xo-flatstring
    cmake --build xo-flatstring/.build -j
    cmake --build xo-flatstring/.build -- test   # run unit tests, cmake invokes ctest
    (cd xo-flatstring/.build && ctest)           # or invoke ctest directly
    cmake --install xo-flatstring/.build

Build, Test and Install `xo-ratio`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-ratio/.build -S xo-ratio
    cmake --build xo-ratio/.build -j
    cmake --build xo-ratio/.build -- test   # run unit tests, cmake invokes ctest
    (cd xo-ratio/.build && ctest)           # or invoke ctest directly
    cmake --install xo-ratio/.build

Build, Test and Install `xo-unit`:

.. code-block:: bash

    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-unit/.build -S xo-unit
    cmake --build xo-unit/.build -j
    cmake --build xo-unit/.build -- test   # run unit tests, cmake invokes ctest
    (cd xo-unit/.build && ctest)           # or invoke ctest directly
    cmake --install xo-unit/.build

Build Examples
--------------

To enable building example programs:

.. code-block:: bash

    cd ~/proj/xo
    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DXO_ENABLE_EXAMPLES=1 -B xo-unit/.build -S xo-unit

Run examples from the build directory:

.. code-block:: bash

    ~/proj/xo/xo-unit/.build/example/ex1/xo_unit_ex1
    ~/proj/xo/xo-unit/.build/example/ex2/xo_unit_ex2
    # etc

Build and Install Documentation
-------------------------------

xo-unit documentation has these additional dependencies:

* `doxygen`_ annotation-driven inline documentation
* `sphinx`_ documentation based on ReST files
* `sphinx-rtd-theme`_ popular CSS theme for sphinx
* `breathe`_ make doxygen-generated ingredients available from sphinx

.. _doxygen: https://www.doxygen.nl
.. _sphinx: https://www.sphinx-doc.org
.. _sphinx-rtd-theme: https://pypi.org/project/sphinx-rtd-theme
.. _breathe: https://breathe.readthedocs.io/en/latest

Preamble (assuming ubuntu here):

.. code-block:: bash

    sudo apt-get install doxygen
    sudo apt-get install python3-sphinx
    sudo apt-get install python3-sphinx-rtd-theme
    sudo apt-get install python3-breathe

Build `xo-unit` docs

.. code-block:: bash

    cd ~/proj/xo
    cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -B xo-unit/.build
    cmake --build xo-unit/.build -- docs
    cmake --install xo-unit/.build   # if docs built,  installs to $PREFIX/share/doc/xo_unit/html

Supported compilers
-------------------

* developed with gcc 12.3.0 and gcc 13.2.0;  github CI using gcc 11.4.0 (asof March 2024)
