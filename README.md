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
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DXO_ENABLE_EXAMPLES=1
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

This builds all xo subprojects,  assembles sandbox under `./result`.

```
$ tree -L 1 ./result
./result
├── bin
│   ├── xo-build
│   ├── xo-cmake-config
│   └── xo-cmake-lcov-harness
└── share
    ├── cmake
    │   └── xo_macros
    │       ├── code-coverage.cmake
    │       ├── xo-project-macros.cmake
    │       └── xo_cxx.cmake
    ├── etc
    │   └── xo
    │       └── subsystem-list
    └── xo-macros
        ├── Doxyfile.in
        ├── gen-ccov.in
        └── xo-bootstrap-macros.cmake
```

For completeness' sake: can also use
```
$ nix-build -A xo-userenv-slow
```

Same result as `$nix-build -A xo-userenv`, but builds each package serially
using `xo-build`.

### Coverage Build

Prepare build
```
# phase 2
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=ON
```

Build coverage-enabled libraries and executables
```
$ (cd .build && make ccov)
```

## To view docs from WSL

1. find wsl IP address

```
$ hostname -I
```

2. run nix build
```
$ nix-build -A xo-userenv
```

3. serve docs from some available port
```
$ (cd result && python3 -m http.server 3000)
```

4. browse to "<wsl-ip-address>:3000/share/doc/xo-flatstring/html" from windows

## To add a new satellite repo

1. check clone in clean state (all local changes committed or unwound)

2. add satellite as remote

```
$ git remote add xo-foo git@github.com:Rconybea/xo-foo.git
$ git fetch xo-foo
```

3. checkout satellite repo

```
$ git subtree add --prefix=xo-foo xo-foo main
```
