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
