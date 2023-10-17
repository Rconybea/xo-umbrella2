# pybind11 utilities for XO projects

## Getting Started

### build + install dependencies

- see [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

### to build + install locally

```
$ cd xo-pyutil
$ mkdir build
$ cd build
$ PREFIX=/usr/local # for example
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=$(PREFIX) -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make
$ make install
```

### LSP support
```
$ cd xo-pyutil
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
