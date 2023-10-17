# ordinal tree library

## Getting Started

### build + install dependencies

- see [github/Rconybea/randomgen](https://github.com/Rconybea/randomgen) -- random number generators e.g. xoshiro256ss
- see [github/Rconybea/refcnt](https://github.com/Rconybea/refcnt) -- intrusive reference-counting

### build + install
```
$ cd xo-ordinaltree
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```

### build for unit test coverage
```
$ cd xo-ordinaltree
$ mkdir build-ccov
$ cd build-ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

### LSP support
```
$ cd xo-ordinaltree
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
