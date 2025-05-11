# python bindings for c++ reflection library (xo-pyreflect)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)
- [github/Rconybea/xo-reflect](https://github.com/Rconybea/xo-reflect)

### build + install
```
$ cd xo-pyreflect
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

Assumes `xo-pyreflect` installed to `~/local2/lib`

```
PYTHONPATH=~/local2/lib python
>>> import xo_pyreflect
>>> dir(xo_pyreflect)
['SelfTagging', 'TaggedRcptr', 'TypeDescr', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>> xo_pyreflect.TypeDescr.print_reflected_types()
<type_table_v[0]:>
```
(Not _immediately_ interesting:  no reflected types in `pyreflect` itself)

## Development

### build for unit test coverage
```
$ cd xo-pyreflect
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
$ cd xo-pyreflect
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-pyprintjson/build
$ cmake -LAH
```
