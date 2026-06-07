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

## Examples

```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pyprintjson
>>> dir(xo_pyprintjson)
['PrintJson', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>> dir(xo_pyprintjson.PrintJson)
['__class__', '__delattr__', '__dir__', '__doc__', '__eq__', '__format__', '__ge__', '__getattribute__', '__getstate__', '__gt__', '__hash__', '__init__', '__init_subclass__', '__le__', '__lt__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', 'instance', 'print']
>>>
```

## Development

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

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-pyprintjson/build
$ cmake -LAH
```
