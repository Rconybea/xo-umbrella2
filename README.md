# random number generators

# to build + install locally

```
$ cd randomgen
$ mkdir build
$ cd build
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=$(INSTALL_PREFIX) ..
$ make
$ make install
```
