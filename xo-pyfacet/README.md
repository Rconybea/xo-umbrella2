# python bindings for c++ faceted object model (xo-facet)

Exposes [xo-facet](../xo-facet) -- the faceted object model (fomo) -- to
python.  The interesting surface here is the handle machinery: python needs to
hold fomo objects across a gc cycle, which is what `DHandleStore` and
`ObjectHandle` exist to make possible.

Status: **scaffold**.  The module builds, imports and links against
`xo_facet`, but binds nothing yet.

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-facet](https://github.com/Rconybea/xo-facet)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)

### build + install

```
$ cd xo-pyfacet
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

Assumes `xo-pyfacet` installed to `~/local2/lib`
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pyfacet
>>> dir(xo_pyfacet)
['__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>>
```
