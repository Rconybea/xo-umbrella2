# XO cmake modules

Collects cmake macros to be shared across XO projects (e.g. indentlog, reflect, kalman, ..)

## Features

- support for both manyrepo and monorepo projects
- support for generating cmake `xxxConfig.cmake` files,  so cmake `find_package()` works reliably

## Example

In some XO project `foo`:
```
$ cd build
$ cmake -DCMAKE_MODULE_PATH=/usr/local/share/cmake ..
```

then in `foo/CMakeLists.txt`:
```
include(xo_macros/xo_cxx)
```
