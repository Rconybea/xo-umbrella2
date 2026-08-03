# XO cmake modules

Collects cmake macros to be shared across XO projects (e.g. indentlog, reflect, kalman, ..)

## Features

- support for both manyrepo and monorepo projects
- support for generating cmake `xxxConfig.cmake` files,  so cmake `find_package()` works reliably
- support for header-only libraries
- support for pybind11 libraries
- documentation generation using doxygen + breathe + sphinx
- code coverage using ccov + lcov

## Getting Started

### copy repo

```
$ git clone https://github.com:rconybea/xo-cmake.git
```

### configure + install
```
$ cd xo-cmake
$ cmake -DCMAKE_INSTALL_PREFIX=/usr/local -B .build -S .   # ..or desired prefix
$ cmake --install .build
```

## use from a cmake project

In some project `foo`:
```
$ cd foo
$ mkdir cmake
$ cp $PREFIX/share/xo-macros/xo-bootstrap-macros.cmake cmake/
```

`xo-bootstrap-macros-cmake` has two vital jobs:
1. set `XO_CMAKE_CONFIG_EXECUTABLE` (locate `xo-cmake-config`)
2. set `CMAKE_MODULE_PATH` (obtained from `xo-cmake-config --cmake-module-path`)

then in `foo/CMakeLists.txt`:
```
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()
```

Now as long as `$PREFIX/bin` is in `PATH`:
```
$ cd mybuild
$ cmake path/to/foo/source
```

### or set `XO_CMAKE_CONFIG_EXECUTABLE` and `CMAKE_MODULE_PATH`

In some project `foo`:
```
$ cd mybuild
$ cmake -DXO_CMAKE_CONFIG_EXECUTABLE=xo-cmake-config -DCMAKE_MODULE_PATH=$(xo-cmake-config --cmake-module-path) path/to/foo/source
```

## Tools

Installed to `$PREFIX/bin`.

### xo-loc

Treemap of the xo subsystem world: one tile per subsystem, area proportional to
lines of code, color by artifact kind.  Sibling to `xo-deps`.

```
$ xo-loc                              # svg of the whole tree
$ xo-loc --format=html                # interactive: hover for numbers, click to focus
$ xo-loc --color=complexity           # shade Code tiles by complexity per kLOC
$ xo-loc --format=csv                 # the underlying table
$ xo-loc xo-gc xo-arena               # just these subsystems
```

Requires `scc` (>= 3.5.0) on `PATH`; the tree is counted live on every run, so
there is no snapshot file to go stale.  The walk always starts at the source
root — found via `--root`, `$XO_SOURCE_ROOT`, git, then an upward search — so it
works from any directory, and subsystem arguments filter the output rather than
the walk.

By default the map shows hand-written code: vendored third-party code and
generated code are excluded, and **every exclusion is reported on stderr**.  Use
`--include-vendored` and `--include-generated` to put them back.

Tiles are subdivided by kind only when each part is worth drawing: a kind must
be at least 1/8 of its own subsystem, and the resulting rectangle must be big
enough to label and not a splinter.  Anything folded away is reported too.
