# python bindings for c++ arena allocator (xo-arena)

Exposes [xo-arena](../xo-arena)'s **configuration** types to python:
`ArenaConfig`, `AllocHeaderConfig`, and the `AllocHeader` word the latter
describes.

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-arena](https://github.com/Rconybea/xo-arena)
- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)

### build + install

```
$ cd xo-pyarena
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
>>> import xo_pyarena as mm
>>> mm.ArenaConfig(name="scratch", size=64*1024)
<ArenaConfig name='scratch' size=65536 hugepage_z=2097152 store_header_flag=False debug_flag=False>
>>> mm.AllocHeaderConfig()
<AllocHeaderConfig tseq_bits=24 age_bits=8 size_bits=32 guard_z=0>
```

See `xo-pyfacet/examples/flywheel_pretty.py` for these configs in use.
