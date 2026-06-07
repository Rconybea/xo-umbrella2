# random number generators

## Getting Started

### build + install dependencies

- see [github/Rconybea/cmake](https://github.com/Rconybea/xo-cmake) -- cmake modules

### to build + install locally
```
$ cd randomgen
$ mkdir build
$ cd build
$ PREFIX=/usr/local # for example
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=$(PREFIX) -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make
$ make install
```
