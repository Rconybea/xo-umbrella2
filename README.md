# Introduction

Local nix build for xo libraries.
Intended for local development work, with source in immediate subdirectories.

## Features

- native c++
- deterministic simulation
- reflection
- python bindings

## Getting Started

### Nix shell (reproducible development environment)

If `nix` is available, can get several reproducible build environments.

Pick one for a session:

```
$ cd xo-umbrella2
$ nix-shell -A shell0  # just nix stdenv: make,gcc,bash etc.
$ nix-shell -A shell1  # stable environment
$ nix-shell -A shell2  # stable environment + emacs + lsp
$ nix-shell -A shell3  # stable environment + emacs + lsp + xorg/opengl/vulkan/imgui stack (wsl2-only)
$ nix-shell -A shell4  # wsl2-specific. like shell3, vkcube works (at least on WSL)
$ nix-shell -A shell5  # wsl2-specific. uses dxg driver for "hardware acceleration"
$ nix-shell -A shell   # (deprecated) bleeding edge environment
```

Run emacs (for example) from within chosen seesion.
We need this ordering because nix-shell determines vital details like location of shared libraries,
including libraries used by running IDE.

```
$ emacs
```

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
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DXO_ENABLE_EXAMPLES=1 -DCMAKE_BUILD_TYPE=debug
$ cmake --build .build --verbose
$ cmake --install .build
```

or with Vulkan examples
```
# at start of phase 2
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DXO_ENABLE_EXAMPLES=1 -DXO_ENABLE_VULKAN=1
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

#### Nix + Vulkan

Currently (Nov 2025) only affects `xo-umbrella2/xo-imgui`.

Non-trivial, because:
1. must use host OS for gpu drivers.  nixpkgs has drivers, but they're setup to work from nixos.
2. want to use nixpkgs for the GPU-independent portion of graphics stack.

Complication because host gpu drivers in a "big swimming pool" such as `/usr/lib/x86_64-linux-gnu/`
that contains both libraries that must come from host OS (e.g. `libGLX_nvidia`) and libraries
that must come from nixpkgs (e.g. `libc`)

Finesse by introducing a directory-of-symlinks, see `xo-umbrella2/etc/{hostegl, hostubuntu}`.
These currently setup by hand, so likely to need manual attention on another host.

An ordinary cmake build may cheerfully use the host-provided graphics stack,
in return for higher risk of DLL hell.

### Coverage Build

Prepare build
```
# phase 2
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=coverage
```

Run coverage-enabled unit tests
```
$ (cd .build && ctest)
```

Generate coverage report
```
$ .build/gen-ccov
```

Html report left in ``.build/ccov/html/index.html``

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
