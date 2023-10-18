# python bindings for c++ stochastic process library (xo-process)

# build + install
```
$ cd xo-pyprocess
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

# build for unit test coverage
```
$ cd xo-pyprocess
$ mkdir build-ccov
$ cd build-ccov
$ cmake \
    -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake \
    -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} \
    -DCODE_COVERAGE=ON \
    -DCMAKE_BUILD_TYPE=Debug ..
```

# LSP (language server) support

LSP looks for compile commands in the root of the source tree;
while Cmake creates them in the root of its build directory.

```
$ cd xo-pyprocess
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```

# Examples

Assumes `xo-pyprocess` installed to `~/local2/lib`

```
PYTHONPATH=~/local2/lib python
>>> import pyprocess
>>> dir(pyprocess)
```
