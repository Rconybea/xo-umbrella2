.. _install:

.. toctree::
   :maxdepth: 2

Source
======

Souce code lives on github `here`_

.. _here: https://github.com/rconybea/xo-tokenizer

To clone from git:

.. code-block:: bash

    git clone https://github.com/rconybea/xo-tokenizer

Tested with gcc 13.3

Install
=======

``xo-tokenizer`` uses supporting library ``xo-indentlog`` and cmake macros ``xo-cmake``.
These are on github:

- `xo-tokenizer source`_ (Schematika tokenizer)
- `xo-indentlog source`_ (structured logging)
- `xo-cmake source`_ (shared cmake macros)

.. _xo-tokenizer source: https://github.com/rconybea/xo-tokenizer
.. _xo-indentlog source: https://github.com/rconybea/indentlog
.. _xo-cmake source: https://github.com/rconybea/xo-cmake

Installing from source
----------------------

Install scripts for `xo-tokenizer` and `xo-indentlog` depend on helper scripts installed from `xo-cmake`.

Preamble:

.. code-block:: bash

   mkdir -p ~/proj/xo
   cd ~/proj/xo

   git clone https://github.com/rconybea/xo-cmake

   PREFIX=/usr/local  # ..or desired installation prefix

   # want PREFIX/bin in PATH to use xo-cmake helpers
   PATH=$PREFIX/bin:$PATH

Install `xo-cmake`:

.. code-block:: bash

    cmake -B xo-cmake/.build -S xo-cmake
    cmake --build xo-cmake/.build -j    # placeholder, can omit for now
    cmake --install xo-cmake/.build

Install `xo-indentlog`:

.. code-block:: bash

    xo-build --clone --configure --build --install indentlog

Install `xo-tokenizer`:

.. code-block:: bash

    xo-build --clone --configure --build --install xo-tokenizer

Directories under ``PREFIX`` will then contain:

.. code-block::

    PREFIX
    +- bin
    |  +- xo-build
    |  +- xo-cmake-config
    |  \- xo-cmake-lcov-harness
    +- include
    |  \- xo
    |     +- indentlog/
    |     \- tokenizer/
    +- lib
    |  \- cmake
    |     +- indentlog/
    |     \- xo_tokenizer/
    +- share
       \- cmake
          \- xo_macros
             +- Doxyfile.in
             +- gen-ccov.in
             \- xo-bootstrap-macros.cmake

Use CMake Support
-----------------

To use built-in cmake suport, when using ``xo-tokenizer`` from another project:

Make sure ``PREFIX/lib/cmake`` is searched by cmake (if necessary, include it in ``CMAKE_PREFIX_PATH``)

Add to ``CMakeLists.txt``:

.. code-block:: cmake

    FindPackage(xo_tokenizer CONFIG REQUIRED)

    target_link_libraries(mytarget INTERFACE xo_tokenizer)
