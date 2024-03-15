# python bindings for c++ stochastic process library (xo-process)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-process](https://github.com/Rconybea/xo-process)
- [github/Rconybea/xo-reactor](https://github.com/Rconybea/xo-reactor)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)
- [github/Rconybea/xo-pyreflect](https://github.com/Rconybea/xo-pyreflect)
- [github/Rconybea/xo-pyprintjson](https://github.com/Rconybea/xo-pyprintjson)
- [github/Rconybea/xo-pyreactor](https://github.com/Rconybea/xo-pyreactor)

### build + install
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

## Examples

Assumes `xo-pyprocess` installed to `~/local2/lib`
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pyprocess
>>> dir(xo_pyprocess)
['BrownianMotion', 'ExpProcess', 'RealizationSource', 'RealizationTracer', 'StochasticProcess', 'UpxAdapterSink', 'UpxEvent', 'UpxEventStore', 'UpxToConsole', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'make_brownian_motion', 'make_exponential_brownian_motion', 'make_realization_printer', 'make_realization_source', 'make_tracer']
>>>
```

## Development

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
