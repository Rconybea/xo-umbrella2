# python bindings for c++ reflection library (xo-reflect)

### build + install
```
$ cd xo-pyreflect
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer, e.g. ~/local
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```

### build for unit test coverage
```
$ cd xo-pyreflect
$ mkdir build-ccov
$ cd build-ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

### LSP support
```
$ cd xo-pyreflect
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
