# arena library

Arena library with mmap-based superpowers.

Features:
- separate reserve (for address space) and commit (for physical pages) features.
- superpages for larger (> 2MB) extents.
- optional per-alloc header with type hints.
- checkpoint/restore.
- iterator support over allocations.
- supports linux and osx.

Limitations:
- requires =mmap=.

## Documentation

- [documentation](https://rconybea.us/xo-docs/xo-arena)

## Getting started

### Install dependencies

- [github/Rconybea/xo-cmake](https://github.com?Rconybea/xo-cmake) XO cmake macros

Installs cmake ingredients along with build assistant `xo-build` for XO projects such as this one.

### Build + install other XO dependencies

```
$ xo-build --with-deps --clone --configure --build --install xo-arena
```

Note: can use `-n` to dry-run here

### Clone `xo-arena` repository locally

```
$ xo-build --clone xo-arena
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-arena.git
```

### Build and install `xo-arena`

```
$ xo-build --configure --build --install xo-arena
```

or equivalently:

```
$ PREFIX=/usr/local  # for example
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-arena -B xo-arena/.build
$ cmake --build xo-reflect/.build
$ cmake --install xo-reflect/.build
```

### Build for unit test coverage

```
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_INSTALL_PREFIX=$PREFIX xo-arena/.build-ccov
$ cmake --build xo-arena/.build-ccov
```

### LSP support

```
$ cd xo-arena
$ ln -s .build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```

