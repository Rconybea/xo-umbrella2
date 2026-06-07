# python bindings for c++ reflection library (xo-pysimulator)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-simulator](https://github.com/Rconybea/xo-simulator)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)
- [github/Rconybea/xo-pyreflect](https://github.com/Rconybea/xo-pyreflect)
- [github/Rconybea/xo-pyprintjson](https://github.com/Rconybea/xo-pyprintjson)
- [github/Rconybea/xo-pyreactor](https://github.com/Rconybea/xo-pyreactor)

### build + install
```
$ cd xo-pysimulator
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

Assumes `xo-pysimulator` installed to `~/local2/lib`
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pysimulator
>>> dir(xo_pysimulator)
['Simulator', 'SourceTimestamp', 'TimeSlip', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'make_simulator']
>>>

## Development

### build for unit test coverage
```
$ cd xo-pysimulator
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
while `cmake` creates them in the root of its build directory.

```
$ cd xo-pysimulator
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```

## Examples

Assumes `xo-pysimulator` installed to `~/local2/lib`

```
PYTHONPATH=~/local2/lib python
>>> import pysimulator
>>> dir(pysimulator)
```
