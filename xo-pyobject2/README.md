# python bindings for c++ faceted object model (xo-object2)

Will expose [xo-object2](../xo-object2)'s representations -- `DFloat`,
`DInteger`, `DBoolean`, `DList`, `DArray`, `DDictionary`, `DStruct`,
`DRuntimeError` -- to python, as a harness for driving and inspecting the
object model: construct fomo objects, hold them across allocation, render them,
and watch what a handle does to the GC root set.

**Status: scaffold.** The module builds and imports, and registers nothing yet.

## Shape it is being built to

One python class per *representation*, not per facet.  Each class is assembled
from per-facet **binder templates** -- `bind_top<DRepr>(cls)`,
`bind_printable<DRepr>(cls)` -- so a facet's method list is written once and
reused across every representation implementing it, rather than once per
(facet, representation) pair.

That replaces an earlier plan to have `genfacet` emit a handle class per facet.
With `DRepr` known statically inside a pybind translation unit,
`obj<AFacet,DRepr>` is constructible from a bare data pointer with no registry
lookup, so a generated forwarding layer would sit beneath pybind's own.

Objects are held through `DObjectHandle`, which pins a strong root in an
`AllocFlywheel` (see [xo-pyfacet](../xo-pyfacet)) and recovers the typed `obj`
on demand.

Design and tickets: `.xo-backlog/pyobject2/` in the backlog repo; run
`xo-sdlc --milestones` for progress.

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-object2](https://github.com/Rconybea/xo-object2)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)

### build + install

```
$ cd xo-pyobject2
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

From an umbrella build, `xo-python` puts every xo pybind module on `PYTHONPATH`:

```
$ .build/xo-python
>>> import xo_pyobject2
>>> xo_pyobject2.__doc__
'pybind11 plugin for xo.object2'
```
