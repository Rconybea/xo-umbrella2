# Introduction

Local nix build for xo libraries.
Intended for local development work, with source in immediate subdirectories.

## Features

- native c++
- deterministic simulation
- reflection
- python bindings

## Getting Started

(Alternatively, see `xo-umbrella2/docs/install.rst` for similar content)

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

Can use `nix-shell` to get reproducible environment for cmake build, see `Cmake Build`.
Alternatively can use full nix build, see `Nix Build`

### Cmake build

If `nix` is available, you probably prefer the nix build, unless working on XO itself.
Otherwise continue reading..

```
$ cd xo
$ PREFIX=/path/to/say/usr/local
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DXO_ENABLE_EXAMPLES=1 -DXO_ENABLE_DOCS=1 -DCMAKE_BUILD_TYPE=debug
$ cmake --build .build --verbose
$ cmake --install .build
```

or with Vulkan examples
```
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DXO_ENABLE_EXAMPLES=1 -DXO_ENABLE_VULKAN=1
```

### Re-run cmake

```
$ cd xo
$ .build/reconfigure.sh
```

### Cmake build documentation

Documentation relies on doxygen, sphinx and breathe.
```
# phase 2
$ cd xo-umbrella2
$ cmake -B .build -S . -DXO_ENABLE_DOCS=1
$ cmake --build .build -- docs
```

Create Html docs in `.build/sphinx/html/index.html`

### Cmake build with coverage

Prepare build
```
# phase 2
$ cmake -B .build -S . -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=coverage -DENABLE_TESTING=1
$ cmake --build .build --
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

#### Nix + SDL2 + Vulkan + ImGui

Currently (Nov 2025) only affects `xo-umbrella2/xo-imgui`.

For OSX, imgui works with assistance from motlenvk.
Build `xo-imgui` with

```
$ cd xo-umbrella2
$ nix-shell -A shell4-osx   # see xo-umrbella2/default.nix for impl
```

For linux and/or wsl build need extra care:
1. must use host OS for gpu drivers.  nixpkgs has drivers, but they're setup to work from nixos.
2. want to use nixpkgs for the GPU-independent portion of graphics stack.

Complication because host gpu drivers in a "big swimming pool" such as `/usr/lib/x86_64-linux-gnu/`
that contains both libraries that must come from host OS (e.g. `libGLX_nvidia`) and libraries
that must come from nixpkgs (e.g. `libc`)

Finesse by introducing a directory-of-symlinks, see `xo-umbrella2/etc/{hostegl, hostubuntu}`.
These currently setup by hand, so likely to need manual attention on another host.

An ordinary cmake build may cheerfully use the host-provided graphics stack,
in return for higher risk of DLL hell.

To build for ubuntu with nvidia gpu:
```
$ cd xo-umbrella2
$ nix-shell -A shell4-nvidia
# then regular cmake:
$ cmake -B .build -S . -DXO_ENABLE_VULKAN=1 -DXO_ENABLE_EXAMPLES=1 -DCMAKE_INSTALL_PREFIX=$PREFIX
```

To build for wsl:
```
$ cd xo-umbrella2
$ nix-shell -A shell4-wsl
# then regular cmake:
$ cmake -B .build -S . -DXO_ENABLE_VULKAN=1 -DXO_ENABLE_EXAMPLES=1 -DCMAKE_INSTALL_PREFIX=$PREFIX
```

Currently not supporting a nix sandbox build for xo-imgui

## Directory Layout

(not in alphabetical order)

```
xo-umbrella2/
|
+- CMakeLists.txt                 top-level cmake config
+- cmake
|  \- xo-bootstrap-macros.cmake   configure xo cmake support for build
+- compile_commands.json          symlink to path/to/build/compile_commands.json for LSP
|
+- conf.py                        sphinx config for project documentation
+- index.rst                      root of xo-umbrella2 doc tree
+- Doxyfile.in                    doxygen config template; cmake will prepare Doxyfile in build dir
+- _static/                       static inputs to sphinx
|
+- etc
|  +- hostegl/                    sample video driver symlinks for WSL2
|  \- hostubuntu/                 sample video driver symlinks for ubuntu
|
+- default.nix                    top-level nix build (works w/ stock nixpkgs 25.05)
+- pkgs/                          per-satellite nix builds. see xo-umbrella2/default.nix
|  +- xo-callback.nix             nix build for xo-umbrella2/xo-callback
|  ..etc..
|  \- xo-webutil.nix
|
+- xo-alloc/                      xo-alloc subproject. independent git repo, using git subtree
+- xo-alloc2/                     xo-alloc2 subproject.
..etc..
\- xo-webutil/

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
$ git fetch xo-foo main
```

3. checkout satellite repo

```
$ git subtree add --prefix=xo-foo xo-foo main
```
