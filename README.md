# python bindings for c++ webutil library (xo-webutil)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)
- [github/Rconybea/xo-webutil](https://github.com/Rconybea/xo-webutil)

### build + install

```
$ cd xo-pywebutil
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

## Examples

Assumes `xo-pywebutil` installed to `~/local2/lib`
```
$ PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pywebutil
>>> dir(xo_pywebutil)
['EndpointDescr', 'StreamEndpointDescr', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>> from xo_pywebutil import *
```

## Development

### build for unit test coverage
```
$ cd xo-pywebutil
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
$ cd xo-pywebutil
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-pywebutil/build
$ cmake -LAH
```
