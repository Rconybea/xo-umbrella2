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
    $ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=$PREFIX
    $ cmake --build .build -j

    # optionally build docs
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


LSP Setup
=========

To setup xo-umbrella2 build to work with a language server:

.. code-block::

    $ cd xo-umbrella2
    $ ln -s .build /compile_commands.json  # lsp will look for compile_commands.json in project root


In this case subsystem LSP setup should be omitted, git root is ``path/to/xo-umbrella2``,
not ``path/to/xo-umbrella2/xo-ratio`` etc.
