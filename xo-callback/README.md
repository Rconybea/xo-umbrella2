# callback-set with reentrant invocation

Reentrant:
1. A callback can modify parent callback-set (for example to remove itself),
   even while being invoked.
2. Any such re-entrant operations are deferred until callback invocation completes.

## Getting Started

### build + install dependencies

- [github/Rconybea/refcnt](https://github.com/Rconybea/refcnt)

### build + install

```
$ cd xo-callback
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
$ cd xo-callback
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
cmake creates them in the root of its build directory.
```
$ cd xo-callback
$ ln -s build/compile_commands.json
```
