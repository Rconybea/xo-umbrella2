# python bindings for c++ pretty-printing library (xo-indentlog2)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-indentlog2](https://github.com/Rconybea/xo-indentlog2)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)

### build + install

```
$ cd xo-pyindentlog2
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

## Examples

Assumes `xo-pyindentlog2` installed to `~/local2/lib`
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pyindentlog2
>>> dir(xo_pyindentlog2)
['__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>>
```
