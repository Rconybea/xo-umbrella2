.. _install:

.. toctree
   :maxdepth: 2

Source
======

Source code for XO is on github `here`_

.. _here: https://github.com/rconybea/xo-umbrella2

This repo contains source to all the XO libraries,  in the most convenient form for
building + installing all XO libraries, or developing changes to XO itself.

Install
=======

This section explains how to build and install all XO libraries at once.
If you only want to install a subset of XO, refer to build instructions for individual subsystems.
You can build XO using cmake or nix.

Dependencies
------------

XO dependencies (if you're using nix, it will assemble these for you)

- cmake
- llvm
- libwebsockets
- jsoncpp
- eigen
- catch2
- pybind11

XO documentation requires:

- doxygen
- graphviz
- sphinx
- sphinx-rtd-theme
- breathe
- sphinxcontrib-ditaa
- sphinxcontrib-plantuml
- pilllow

cmake build
-----------

The cmake build has two phases.
The first phase bootstraps some generated helper scripts used in the second phase.

.. code-block::

    $ git clone https://github.com/rconybea/xo-umbrella2
    $ cd xo-umbrella2
    $ PREFIX=/usr/local  # for example

    # phase 1 -- install helper scripts
    $ cmake -B .build0 -S xo-cmake -DCMAKE_INSTALL_PREFIX=$PREFIX
    $ cmake --install .build0

    # phase 2 -- build XO, using helpers installed in phase 1.

    # -DXO_ENABLE_DOCS=1 : builds documentation; or 0 to skip
    # -DXO_ENABLE_EXAMPLES=1 : builds example; or 0 to skipos
    # -DXO_ENABLE_VULKAN=1 : builds vulkan-dependent graphics pipeline for imgui; or 0 to skip

    $ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=$PREFIX -DXO_ENABLE_DOCS=1 -DXO_ENABLE_EXAMPLES=1 -DXO_ENABLE_VULKAN=1
    $ cmake --build .build -j

    # optionally build docs (requires -DXO_ENABLE_DOCS in phase2 config)
    $ cmake --build .build -- docs

    $ cmake --install .build


nix build
---------

Nix build uses toplevel ``default.nix``, along with ``pkgs/xo-foo.nix`` for each subsystem ``foo``.
It ignores toplevel ``CMakeLists.txt``

Build XO libraries, assemble sanbox under ``./result``

.. code-block::

    $ nixbuild -A xo-userenv


Aternatively can enter nix environment, then follow instructions for cmake build:

.. code-block::

    $ nix-shell
    $ /nix/store/<hash>-doxygen-1.10.0/bin/doxygen
    # etc


coverage build
--------------

See ``Test Coverage Setup`` under ``Development`` below

Development
===========

Environment
-----------

If nix is available, can use `nix-shell` to get a reproducible development environment for XO work.
Run `nix-shell` from the top-level `xo-umbrella2` directory.

.. list-table::
    :header-rows: 1
    :widths: 20 60 30

   * - command
     - sufficient for
     - platform
   * - `nix-shell -A shell1a`
     - `cmake -DXO_ENABLE_DOCS=0 -DXO_ENABLE_OPENGL=0 -DXO_ENABLE_VULKAN=0`
     -
   * - `nix-shell -A shell1`
     - `cmake -DXO_ENABLE_DOCS=1`
     -
   * - `nix-shell -A shell3`
     - `cmake -DXO_ENABLE_OPENGL=1`
     -
   * - `nix-shell -A shell4-wsl`
     - `cmake -DXO_ENABLE_VULKAN=1`
     - wsl2 on windows11
   * - `nix-shell -A shell4-nvidia`
     - `cmake -DXO_ENABLE_VULKAN=1`
     - nvidia GPU on linux
   * - `nix-shell -A shell4-osx`
     - `cmake -DXO_ENABLE_VULKAN=1`
     - mac osx

For example

.. code-block::

    $ cd xo-umbrella2
    $ nix-shell -A shell4-nvidia
    $ echo $VK_ICD_FILENAMES
    /usr/share/vulkan/icd.d/nvidia_icd.json   # point to library to use for nvidia gpu on this host
    $ which doxygen
    /home/roland/nixroot/nix/store/cb78mifxvic291rcb2qlbpxgl29f5pzf-doxygen-1.13.2/bin/doxygen
    # etc.

LSP Setup
---------

To setup xo-umbrella2 build to work with a language server:

.. code-block::

    $ cd xo-umbrella2
    $ ln -s .build /compile_commands.json  # lsp will look for compile_commands.json in project root


In this case subsystem LSP setup should be omitted, git root is ``path/to/xo-umbrella2``,
not ``path/to/xo-umbrella2/xo-ratio`` etc.

Test Coverage Setup
-------------------

To setup a unit test coverage build/ccov/all-merged

.. code-block::

    # can reuse phase 1 cmake-macros-install

    # phase 2
    $ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=coverage

Then run unit tests

    $ (cd .build && ctest)

To build coverage report

    $ (.build/gen-ccov)

Html report in ``.build/ccov/html/index.html``


Sphinx Autobuild Setup
----------------------

To serve cache-busting headers

.. code-block::

    $ cd xo-umbrella2
    $ sphinx-autobuild . .build/sphinx/html --port 3000
