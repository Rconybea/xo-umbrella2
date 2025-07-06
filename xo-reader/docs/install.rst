.. _install:

.. toctree::
   :maxdepth: 2

Source
======

Source code lives on github `here`_

.. _here: https://github.com/rconybea/xo-reader

To clone from git:

.. code-block:: bash

    git clone https://github.com/rconybea/xo-reader

Tested with gcc 13.3

Install
=======

One-step Install
----------------

Install along with the rest of *XO* from `xo-umbrella2 source`_

.. _xo-umbrella2 source: https://github.com/rconybea/xo-umbrella2

Minimal Dependencies
--------------------

``xo-reader`` uses several supporting libraries from the *XO* project:

- `xo-expression source`_ (Schematika AST representation)
- `xo-tokenizer source`_ (Schematika lexer)
- `xo-reflect source`_ (reflection library)
- `xo-refcnt source`_ (reference-counting library)
- `xo-indentlog source`_ (structured logging)
- `xo-subsys source`_ (utility library)
- `xo-cmake source`_ (shared cmake macros)

.. _xo-expression source: https://github.com/rconybea/xo-expression
.. _xo-tokenizer source: https://github.com/rconybea/xo-tokenizer
.. _xo-reflect source: https://github.com/rconybea/xo-reflect
.. _xo-refcnt source: https://github.com/rconybea/refcnt
.. _xo-indentlog source: https://github.com/rconybea/indentlog
.. _xo-subsys source: https://github.com/rconybea/subsys
.. _xo-cmake source: https://github.com/rconybea/xo-cmake

Installing from source
----------------------

Install scripts for XO libraries depend on helper scripts installed from `xo-cmake`.

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
    cmake --install xo-cmake/.build

Install dependencies in topological order:

.. code-block:: bash

    xo-build --clone --configure --build --install xo-indentlog
    xo-build --clone --configure --build --install xo-subsys
    xo-build --clone --configure --build --install xo-refcnt
    xo-build --clone --configure --build --install xo-reflect
    xo-build --clone --configure --build --install xo-expression
    xo-build --clone --configure --build --install xo-tokenizer
    xo-build --clone --configure --build --install xo-reader

Directories under ``PREFIX`` will then contain:

.. code-block::

    PREFIX
    +- bin
    |  +- xo-build
    |  +- xo-cmake-config
    |  \- xo-cmake-lcov-harness
    +- include
    |  \- xo
    |     +- cxxutil/
    |     +- expression/
    |     +- indentlog/
    |     +- reader/
    |     +- refcnt/
    |     +- reflect/
    |     +- subsys/
    |     \- tokenizer/
    +- lib
    |  +- cmake
    |  |  +- indentlog/
    |  |  +- refcnt/
    |  |  +- reflect/
    |  |  +- subsys/
    |  |  +- xo_expression/
    |  |  +- xo_reader/
    |  |  \- xo_tokenizer/
    |  +- lib*.so
    +- share
       +- cmake
       |  \- xo_macros
       |     +- code-coverage.cmake
       |     +- xo-project-macros.cmake
       |     \- xo_cxx.cmake
       +- etc
       |  \- xo
       |     \- subsystem-list
       \- xo-macros
          +- Doxyfile.in
          +- gen-ccov.in
          \- xo-bootstrap-macros.cmake

CMake Support
-------------

To use built-in cmake support, when using ``xo-reader`` from another project:

Make sure ``PREFIX/lib/cmake`` is searched by cmake (if necessary, include it in ``CMAKE_PREFIX_PATH``)

Add to your ``CMakeLists.txt``:

.. code-block:: cmake

    FindPackage(xo_reader CONFIG REQUIRED)

    target_link_libraries(mytarget INTERFACE xo_reader)
