.. _install:

.. toctree
   :maxdepth: 2

Source
======

Source code lives on github `here`_

.. _here: https://github.com/rconybea/xo-ratio

To clone from git:

.. code-block:: bash

    git clone https://github.com/rconybea/xo-ratio

Implementation relies on c++20 features (expanded use of constexpr; class-instances as template arguments).
Tested with gcc 13.2

Install
=======

Since xo-ratio is header-only, can incorporate into another project just by copying the include directories
to somewhere convenient.

Copy includes
-------------

.. code-block:: bash

    # For example..
    cd myproject
    mkdir -p ext/xo-ratio
    rsync -a -v path/to/xo-ratio/include/ ext/xo-ratio/

Include as git submodule
------------------------

.. code-block:: bash

    cd myproject
    git submodule add -b main https://github.com/rconybea/xo-ratio ext/xo-ratio
    git submodule update --init

This assumes you organize directly-incorporated dependencies under directory ``myproject/ext``.
You would then add ``myproject/ext/xo-ratio/include`` to your compiler's include path,
and from c++ do something like

.. code-block:: c++

    #include <xo/ratio/ratio.hpp>

in c++ source files that rely on xo-ratio

Supported compilers
-------------------

* developed with gcc 13.2.0;  github CI using gcc 11.4.0 (asof April 2024)

Building from source
--------------------

Although the xo-ratio library is header-only, unit tests have some dependencies.
Example instructions (github CI) for build starting from stock ubuntu are in `ubuntu-main.yml`_

.. _ubuntu-main.yml: https://github.com/Rconybea/xo-ratio/blob/main/.github/workflows/ubuntu-main.yml

Unit test dependencies:

* `catch2`_ header-only unit-test framework
* `xo-cmake`_ cmake macros
* `xo-indentlog`_ logging with call-structure indenting
* `xo-randomgen`_ fast random-number generator (xoshiro256ss)

.. _catch2: https://github.com/catchorg/Catch2
.. _xo-cmake: https://github.com/rconybea/xo-cmake
.. _xo-indentlog: https://github.com/rconybea/indentlog
.. _xo-randomgen: https://github.com/rconybea/randomgen

To build documentation, will also need:

* `doxygen`_ c++ documentation
* `graphviz`_ graph drawing software, used by doxygen
* `sphinx`_ documentation system, ReST (`.rst`) format
* `breathe`_ use `doxygen`-generated ingredients in sphinx `.rst` files
* `sphinx_rtd_theme`_ css for sphinx html output

Can install these

.. _doxygen: https://www.doxygen.nl
.. _graphviz: https://graphviz.org
.. _sphinx: https://www.sphinx-doc.org/en/master
.. _breathe: https://breathe.readthedocs.io/en/latest
.. _sphinx_rtd_theme: https://github.com/readthedocs/sphinx_rtd_theme
