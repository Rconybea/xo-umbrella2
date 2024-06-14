# python bindings for llvm JIT for EGAD (xo-pyjit)

## Getting Started

### Build + install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### build + install other necessary XO dependencies
```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
$ xo-build --clone --configure --build --install xo-reflect
$ xo-build --clone --configure --build --install xo-expression
$ xo-build --clone --configure --build --install xo-jit
$ xo-build --clone --configure --build --install xo-pyutil
$ xo-build --clone --configure --build --install xo-pyexpression
```
### copy `xo-pyjit` repository locally
```
$ xo-build --clone xo-pyjit
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-pyjit.git
```

### build + install xo-pyjit
```
$ xo-build --configure --build --install xo-pyjit
```

or equivalently:

```
$ PREFIX=/usr/local         # or preferred install location
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -S xo-pyjit -B xo-pyjit/.build
$ cmake --build xo-pyjit/.build -j
$ cmake --install xo-pyjit/.build
```
(also see .github/workflows/main.yml)

## Examples

Assumes `xo-pyjit` installed to `~/local2/lib`,
i.e. built with `PREFIX=~/local2`.
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> from xo_pyexpression import *
>>> from xo_pyjit import *
>>> x=make_constant(3.14159)
>>> jit=Jit.make()
>>> code=jit.codegen(x)
>>> x.print
double 3.141600e+00
```

## Development

### use from build tree

Limited utility: requires that supporting libraries (e.g. `xo_pyexpression`) appear in PYTHONPATH
```
$ cd xo-pyjit/.build/src/pyjit
$ python
>>> import xo_pyjit
```

### build for unit test coverage
```
$ cd xo-pyexpression
$ cmake -DCMAKE_BUILD_TYPE=coverage -DENABLE_TESTING=on -S . -B .build-ccov
$ cmake --build .build-ccov -j
```

### LSP (language server) support

LSP looks for compile commands in the root of the source tree;
while Cmake creates them in the root of its build directory.

```
$ cd xo-pyexpression
$ ln -s .build/compile_commands.json  # supply compile commands to LSP
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-pyjit/.build
$ cmake -LAH
```
