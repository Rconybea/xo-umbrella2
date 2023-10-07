# pybind11 utilities for XO projects

# to build + install locally

```
$ cd xo-pyutil
$ mkdir build
$ cd build
$ PREFIX=/usr/local # for example
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=$(PREFIX) -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make
$ make install
```
