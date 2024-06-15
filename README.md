# reflection library

## Getting Started

### build + install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### build + install other XO dependencies
```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
```

Note: can use `-n` to dry-run here

### copy `xo-reflect` repository locally
```
$ xo-build --clone xo-reflect
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-reflect.git
```

### build + install xo-reflect
```
$ xo-build --configure --build --install xo-reflect
```

or equivalently:

```
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-reflect -B xo-reflect/.build
$ cmake --build xo-reflect/.build
$ cmake --install xo-reflect/.build
```

### build for unit test coverage
```
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_INSTALL_PREFIX=$PREFIX xo-reflect/.build-ccov
$ cmake --build xo-reflect/.build-ccov
```

### LSP support
```
$ cd xo-reflect
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
