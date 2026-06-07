# schematica tokenizer library

## Getting Started

### build + install 'xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### build + install other required XO dependencies
```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
$ xo-build --clone --configure --build --install xo-reflectutil
```

Note: can use `-n` to dry-run here

### copy `xo-tokenizer` repository locally
```
$ xo-build --clone xo-tokenizer
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-tokenizer.git
```

### build + install `xo-tokenizer`

```
$ xo-build --configure --build --install xo-tokenizer
```

or equivalently:

```
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-tokenizer -B xo-tokenizer/.build
$ cmake --build xo-tokenizer/.build
$ cmake --install xo-tokenizer/.build
```

### build for unit test coverage
```
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_INSTALL_PREFIX=$PREFIX xo-tokenizer/.build-ccov
$ cmake --build xo-tokenizer/.build-ccov
```

### LSP support
```
$ cd xo-tokenizer
$ ln -s .build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
