# xo-object library

A library for scaffolding an object hierarchy for dynamic typing.
Using this for interpreter integration with schematica

## Getting Started

### build + install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### build + install other necessary XO dependencies
```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
$ xo-build --clone --configure --build --install xo-reflect
```

Note: can use `xo-build -n` to dry-run here

### copy `xo-object` repository locally
```
$ xo-build --clone xo-object
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-object.git
```

### build + install xo-object
```
$ xo-build --configure --build --install xo-object
```

or equivalently:
```
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-object -B xo-object/.build
$ cmake --build xo-object/.build
$ cmake --install xo-object/.build
```

### build for unit test coverage
```
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_INSTALL_PREFIX=$PREFIX xo-object/.build-ccov
$ cmake --build xo-object/.build-ccov
```

### LSP support
```
$ cd xo-object
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
