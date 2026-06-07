# python bindings for Egad abstract syntax tree library (xo-expression)

## Getting Started

### Build + install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### Build + install other necessary XO dependencies

```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
$ xo-build --clone --configure --build --install xo-reflect
$ xo-build --clone --configure --build --install xo-expression
$ xo-build --clone --configure --build --install xo-jit
$ xo-build --clone --configure --build --install xo-pyutil
```

### copy `xo-pyexpression` repository locally
```
$ xo-build --clone xo-pyexpression`
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-pyexpression.git
```

### build + install xo-pyexpression
```
$ xo-build --configure --build --install xo-pyexpression
```

or equivalently:
```
$ PREFIX=/usr/local         # or preferred install location
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -S xo-pyexpression -B xo-pyyexpression/.build
$ cmake --build xo-pyexpression/.build -j
$ cmake --install xo-pyexpression/.build
```
(also see .github/workflows/main.yml)

## Examples

Assumes `xo-pyexpression` installed to `~/local2/lib`,
i.e. built with `PREFIX=~/local2`.
```
PYTHONPATH=~/locasl2/lib:$PYTHONPATH python
>>> import xo_pyexpression
>>> dir(xo_pyexpression)
['Expression', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'exprtype']
```

## Development

### use from build tree
Requires that supporting libraries (e.g. `xo_pyreflect`) appear in PYTHONPATH
```
$ cd xo-pyexpression/.build/src/pyexpression
$ python
>>> import xo_pyexpression
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
$ cd xo-pyexpression/.build
$ cmake -LAH
```
