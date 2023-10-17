# XO cmake modules

Collects cmake macros to be shared across XO projects (e.g. indentlog, reflect, kalman, ..)

## Features

- support for both manyrepo and monorepo projects
- support for generating cmake `xxxConfig.cmake` files,  so cmake `find_package()` works reliably

## Example

In some XO project `foo`:
```
$ cd build
$ PREFIX=/usr/local # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make install
```

then in `foo/CMakeLists.txt`:
```
include(xo_macros/xo_cxx)
```

when configuring `foo`:
```
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake path/to/foo
```
