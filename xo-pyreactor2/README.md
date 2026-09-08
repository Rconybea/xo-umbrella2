# python bindings for c++ reactor library (xo-reactor2)

Counterpart to [xo-pyreactor](../xo-pyreactor), which binds the older
`xo-reactor`.  This module binds `xo-reactor2` -- the facet-based
(`obj<AFacet>`) reactor -- and is expected to replace `xo-pyreactor` once that
surface settles.

Status: **scaffold**.  The module builds, imports and links against
`xo_reactor2`, but binds nothing yet.

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-reactor2](https://github.com/Rconybea/xo-reactor2)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)

### build + install

```
$ cd xo-pyreactor2
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

Assumes `xo-pyreactor2` installed to `~/local2/lib`
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pyreactor2
>>> dir(xo_pyreactor2)
['__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>>
```
