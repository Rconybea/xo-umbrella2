.. _internals:

.. toctree
   :maxdepth: 2

Internals
=========

Monorepo
--------

The umbrella project ``xo-umbrella2`` provides a monorepo with all the XO projects.
Each XO project appears as a top-level subdirectory (``xo-umbrella2/xo-arena``, ``xo-umbrella2/xo-flatstring``, ..).
Each project also has it's own satellite repo, and can build/install independently of xo-umbrella2.

In ``xo-umbrella2`` We use `git-subrepo`_ to coordinate the monorepo and each satellite repo.
The subrepo feature rewrites commits in each direction.
For example an ``xo-umbrella2`` commit touching ``xo-foo/src/foo/source.cpp`` will have a parallel ``xo-foo`` commit
touching ``src/foo/source.cpp``.

.. _git-subrepo: https://github.com/ingydotnet/git-subrepo

There are some procedural differences between monorepo and satellite builds that are salient to cmake.
We use the ``XO_SUBMODULE_BUILD`` to trigger monorepo-specific behavior.

XO Cmake Macros
---------------

XO projects rely on a common set of cmake macros.
Typical practice for a project ``foo`` using cmake would be to collect private macros in ``foo/cmake/``,
then refer to them from ``foo/CMakeLists.txt``.

XO uses a different policy, because we want to use the same set of cmake macros across all XO satellite projects,
but we don't want to maintain separate copies. Instead we put XO cmake macros in their own repo
``xo-cmake``.  Instead of maintaining a copy of the full XO cmake macro set, each satellite project contains
its own copy of a common cmake bootstrap sequence. The bootstrap's job is to add the location of ``xo-cmake``
to ``CMAKE_MODULE_PATH``.

In a monorepo build, the XO cmake macros are available at ``xo-umbrella2/xo-cmake``.

This gives us two ways to get XO cmake macros:

- in-tree.
  Build with ``CMAKE_MODULE_PATH=`` or ``CMAKE_MODULE_PATH=prefix`` (literal ``prefix`` here)

- installed.
  Build with ``CMAKE_MODULE_PATH=$PREFIX/share/cmake`` where PREFIX is installed location of ``xo-cmake``.

The bootstrap sequence needs to know whether it's runing on behalf of an XO monorepo or an indepndent satellite
project.  We use cmake variable ``XO_SUBMODULE_BUILD`` to signal this.

Documentation
-------------

In XO projects we use a combination of doxygen, sphinx and breathe to assemble documentation.
An individual satellite project will have a sphinx entry point ``xo-foo/doc/conf.py``,
targeting building a documentation tree for ``xo-foo`` alone.

In a monorepo build we want to generate a consolidated documentation tree.
This relies on some monorepo-only scaffolding:

- xo-umbrella2/conf.py.
  Sphinx configuration file

- xo-umbrella2/Doxyfile.in
  CMake template for ``$BUILDDIR/Doxyfile``

- index.rst.
  Top-level sphinx documentation page.
  This page points to ``xo-foo/docs/index`` for each satellite ``xo-foo`` that provides
  sphinx documentation.

- docs/.
  Additional umbrella-only sphinx documentation
  
Continuous Integration
----------------------

The primary CI pipeline runs on forgejo, see ``.forgejo/workflows/ci.yaml``.
The forgejo setup uses a custom runner so that the nix store is shared between builds.

There's also a github pipeline, see ``.github/workflows/main-stock-ubuntu.yml``.
In practice that's proved more awkward to maintain, since it installs 3rd party dependencies
on top of stock ubuntu.









