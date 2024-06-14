# xo-expression library

A library for representing abstract syntax trees for EGAD (a small expression-based language).

## Getting Started

### build + install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### build + install other XO dependencies
```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
$ xo-build --clone --configure --build --install xo-reflect
```

Note: can use `xo-build -n` to dry-run here

### copy `xo-expression` repository locally
```
$ xo-build --clone xo-expression
```

or eqivalently
```
$ git clone git@github.com:Rconybea/xo-expression.git
```

### build + install xo-expression
```
$ xo-build --configure --build --install xo-expression
```

or equivalently:
```
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-expression -B xo-jit/.build
$ cmake --build xo-expression/.build
$ cmake --install xo-expression/.build
```

### build for unit test coverage
```
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_INSTALL_PREFIX=$PREFIX xo-expression/.build-ccov
$ cmake --build xo-expression/.build-ccov
```

### LSP support
```
$ cd xo-expression
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
