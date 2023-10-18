# python bindings for c++ printjson library (xo-printjson)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)
- [github/Rconybea/xo-printjson](https://github.com/Rconybea/xo-printjson)

### build + install

```
$ cd xo-pyprintjson
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer, e.g. ~/local
$ cmake \
    -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake \
    -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```
(also see .github/workflows/main.yml)

### build for unit test coverage
```
$ cd xo-pyprintjson
$ mkdir build-ccov
$ cd build-ccov
$ cmake \
    -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake \
    -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} \
    -DCODE_COVERAGE=ON \
    -DCMAKE_BUILD_TYPE=Debug ..
```

### LSP (language server) support

LSP looks for compile commands in the root of the source tree;
while Cmake creates them in the root of its build directory.

```
$ cd xo-pyprintjson
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```
