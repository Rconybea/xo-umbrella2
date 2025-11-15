.. _install:

.. toctree:
   :maxdepth: 2

Source
======

Source code lives on github `here`_

.. _here: https://github.com/rconybea/xo-interpreter

To clone from git:

.. code-block:: bash

    git clone https://github.com/rconybea/xo-interpreter

Tested with gcc 14.2

Install
=======

One-step Install
----------------

Install xo-interpreter along with the rest of *XO* from `xo-umbrella2 source`_:
see install instructions for xo-umbrella2.

.. _xo-umbrella2 source: https://github.com/rconybea/xo-umbrella2

Essential Xo Dependencies
-------------------------

``xo-interpreter`` uses several supporting libraries from elsewhere in the *XO* project:

- `xo-reader source`_ (Schematika expression parser)
- `xo-expression source`_ (Schematika AST representation)
- `xo-tokenizer source`_ (Schematika lexer)
- `xo-object source`_ (gc-eligible runtime polymorphism)
- `xo-randomgen source`_ (fast pseudo-random number generators)
- `xo-alloc source`_ (arena allocators, garbage collector)
- `xo-unit source`_ (dimension checking library)
- `xo-ratio source`_ (exact ratio library)
- `xo-flatstring source`_ (no-allocation string library)
- `xo-callback source`_ (callback library)
- `xo-reflectutil source`_ (reflection utils for participating libs)
- `xo-reflect source`_ (reflection library)
- `xo-refcnt source`_ (reference-counting library)
- `xo-subsys source`_ (utility library)
- `xo-indentlog source`_ (structured logging, pretty-printing)
- `xo-cmake source`_ (shared cmake macros)

.. _xo-reader source: https://github.com/rconybea/xo-reader
.. _xo-expression source: https://github.com/rconybea/xo-expression
.. _xo-tokenizer source: https://github.com/rconybea/xo-tokenizer
.. _xo-object source: https://github.com/rconybea/xo-object
.. _xo-randomgen source: https://github.com/rconybea/xo-randomgen
.. _xo-alloc source: https://github.com/rconybea/xo-alloc
.. _xo-unit source: https://github.com/rconybea/xo-unit
.. _xo-ratio source: https://github.com/rconybea/xo-ratio
.. _xo-flatstring source: https://github.comr/rconybea/xo-flatstring
.. _xo-callback source: https://github.com/rconybea/xo-callback
.. _xo-reflect source: https://github.com/rconybea/xo-reflect
.. _xo-refcnt source: https://github.com/rconybea/refcnt
.. _xo-subsys source: https://github.com/rconybea/subsys
.. _xo-indentlog source: https://github.com/rconybea/indentlog
.. _xo-cmake source: https://github.com/rconybea/xo-cmake

Building from source
--------------------

Instructions for building xo-interpreter from source, along with only its essential dependencies.

Install scripts for XO libraries depend on helper scripts installed from `xo-cmake`.

Preamble:

.. code-block:: bash

    mkdir -p ~/proj/xo
    cd ~/proj/xo

    git clone https://github.com/rconybea/xo-cmake

    PREFIX=$HOME/local  # or desired installation path

    # will want PREFIX/bin in PATH to use xo-cmake helpers
    PATH=$PREFIX/bin:$PATH

Isntall `xo-cmake`:

.. code-block:: bash

    cmake -B xo-cmake/.build -S xo-cmake
    cmake --install xo-cmake/.build

Now that we have xo-build in PATH, can build+install XO components in topological order:

.. code-block:: bash

    xo-build --clone --configure --build --install xo-indentlog
    xo-build --clone --configure --build --install xo-subsys
    xo-build --clone --configure --build --install xo-refcnt
    xo-build --clone --configure --build --install xo-reflect
    xo-build --clone --configure --build --install xo-reflectutil
    xo-build --clone --configure --build --install xo-callback
    xo-build --clone --configure --build --install xo-flatstring
    xo-build --clone --configure --build --install xo-ratio
    xo-build --clone --configure --build --install xo-unit
    xo-build --clone --configure --build --install xo-alloc
    xo-build --clone --configure --build --install xo-randomgen
    xo-build --clone --configure --build --install xo-object
    xo-build --clone --configure --build --install xo-tokenizer
    xo-build --clone --configure --build --install xo-expression
    xo-build --clone --configure --build --install xo-reader

Directories under ``PREFIX`` will then contain something like:

.. code-block::

    PREFIX
    += bin
    |  +- xo-build
    │  +- xo-cmake-config
    │  \- xo-cmake-lcov-harness
    +─ include
    |  \- xo
    │     +- alloc/
    |     +- callback/
    |     +- cxxutil/
    |     +- expression/
    |     |  +- typeinf/
    |     |  ..
    |     +- flatstring/
    |     +- indentlog/
    |     |  +- machdep/
    |     |  +- print/
    |     |  +- timeutil/
    |     |  ..
    |     +- object/
    |     +- randomgen/
    |     +- ratio/
    |     +- reader/
    |     +- refcnt/
    |     +- reflect/
    |     |  +- atomic/
    |     |  +- function/
    |     |  +- pointer/
    |     |  +- struct/
    |     |  +- vector/
    |     |  ..
    |     +- reflectutil/
    |     +- subsys/
    |     +- tokenizer/
    |     \- unit/
    +- lib
    |  +- cmake
    |  |  +- callback/
    |  |  +- indentlog/
    |  |  +- randomgen/
    |  |  +- refcnt/
    |  |  +- reflect/
    |  |  +- subsys/
    |  |  +- xo_alloc/
    |  |  +- xo_expression/
    |  |  +- xo_flatstring/
    |  |  +- xo_object/
    |  |  +- xo_ratio/
    |  |  +- xo_reader/
    |  |  +- xo_reflectutil/
    |  |  +- xo_tokenizer/
    |  |  \- xo_unit/
    |  +- librefcnt.so
    |  ..
    \- share
       +- cmake
       |  \- xo-macros
       |     +- code-coverage.cmake
       |     +- xo_cxx.cmake
       |     \- xo-project-macros.cmake
       +- etc
       |  \- xo
       |     \- subsystem_list
       +- xo-macros
          +- Doxyfile.in
          +- gen-ccov.in
          \- xo-bootstrap-macros.cmake

CMake Support
-------------

To use built-in cmake support, when using ``xo-interpreter`` from another project:

Make sure ``PREFIX/lib/cmake`` is searched by cmake (for example include it in ``CMAKE_PREFIX_PATH``)

Add to your ``CMakeLists.txt``:

.. code-block:: cmake

    FindPackage(xo_interpreter CONFIG REQUIRED)
    target_link_libraries(mytarget PUBLIC xo_interpreter)
