# printjson library

## Getting Started

### build + install dependencies

- [github/Rconybea/reflect](https://github.com/Rconybea/reflect)

### build + install

```
$ cd xo-printjson
$ mkdir build
$ cd build
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${PREFIX} -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make
$ make install
```

### build for unit test coverage

```
$ cd xo-printjson
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
$ make               # builds executables
$ make ccov          # runs instrumented unit tests
$ make ccov-all      # generates lcov report
```

### LSP support

```
$ cd xo-printjson
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
