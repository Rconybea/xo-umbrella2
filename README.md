# python bindings for c++ reflection library (xo-distribution)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-pyutil](https://github.com/Rconybea/xo-pyutil)
- [github/Rconybea/xo-reflect](https://github.com/Rconybea/xo-distribution)

### build + install
```
$ cd xo-pydistribution
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

### build for unit test coverage
```
$ cd xo-pydistribution
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
$ cd xo-pydistribution
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```

## Examples

Assumes `xo-pydistribution` installed to `~/local2/lib`

```
PYTHONPATH=~/local2/lib python
>>> import pydistribution
>>> dir(pydistribution)
['Distribution', 'ExplicitDist', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'normalcdf']
>>> from pydistribution import *
```

normal distribution
```
>>> normalcdf(0.0)
0.5
>>> normalcdf(3.0)
0.9986501019683699
```

explicit distribution (online implementation).
intended to model empirically a Bayesian prior.
```
>>> d=ExplicitDist.make(bucket_dx=0.01, ref_value=1e-6)
>>> d
<ExplicitDist :cdf_valid_flag 1 :bucket_dx 0.01 :ref_value 0 :lz 0 :hz 1 :lo_v [] :hi_v "[<ProbabilityBucket :wt 1 :cdf 1>]">
>>> d.cdf(0.0)
0.0
>>> d.cdf(0.01)
1.0
```
