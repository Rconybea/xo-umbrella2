# Introduction

Local nix build for xo libraries.
Intended for local development work, with source in immediate subdirectories.

## Features

- native c++
- deterministic simulation
- reflection
- python bindings

## Getting Started

### Cmake build

If `nix` is available, you probably prefer the nix build.
Otherwise continue reading..

The cmake build has two phases, because it needs to bootstrap
generated `xo-cmake-config`, `xo-build` helpers.

```
$ cd xo
$ PREFIX=/path/to/say/usr/local
# phase 1
$ cmake -B .build0 -S xo-cmake -DCMAKE_INSTALL_PREFIX=${PREFIX}
$ cmake --build .build0
$ cmake --install .build0
# phase 2
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX}
$ cmake --build .build
$ cmake --install .build
```

### Nix Build

Nix build uses toplevel `default.nix`,
along with top-level `pkgs/xo-foo.nix` for each subproject `foo`.
It doesn't interact with toplevel `CMakeLists.txt`.

```
$ nix-build -A xo-userenv
```
