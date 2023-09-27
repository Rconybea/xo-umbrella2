# reflection library

### build + install
```
$ cd reflect
$ mkdir build
$ cd build
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```

### build for unit test coverage
```
$ cd refcnt
$ mkdir build-ccov
$ cd build-ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```
