# ratio library

Header-only, constexpr library providing exact representation for rational numbers.

Relative to `std::ratio`:
1. Uses `constexpr` instead of creating new types.
   This means it can be used seamlessly at runtime.
2. Supports a few more arithmetic operations,
   for example exponentiation to integer powers.
3. Provides constexpr conversion to fixed-capacity strings (using xo-flatstring)
3. Provides concept support (with c++20)
4. Requires modern (c++17) support to achieve this

Relative to `boost::ratio`:
1. Streamlined, assumes modern compiler support

## Documentation

- xo-ratio documentation [under construction]: [documentation](https://rconybea.github.io/web/xo-ratio/html/index.html)
- unit test coverage here: [coverage](https://rconybea.github.io/web/xo-ratio/ccov/html/index.html)

## Getting Started

### install dependencies

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake) cmake macros
- [github/Rconybea/xo-flatstring](https://github.com/Rconybea/xo-flatstring) fixed-capacity strings
- [github/rconybea/xo-indentlog](https://github.com/Rconybea/xo-indentlog) logging (used by unit tests)
- [github/rconybea/xo-randomgen](https://github.com/Rconybea/xo-randomgen) rng (used by unit tests)

### copy repository locally

Using `xo-build` (provided by `xo-cmake`):
```
$ xo-build --clone xo-ratio`
```

or equivalently:
```
$ cd ~/proj   # for example
$ git clone https://github.com/Rconybea/xo-ratio
```

### build + install

Using `xo-build`:
```
$ xo-build --configure --build --install xo-ratio
```

or equivalently:
```
$ PREFIX=/usr/local # for example
$ BUILDDIR=.build   # for example
$ mkdir xo-ratio/${BUILDDIR}
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-ratio -B xo-ratio/${BUILDDIR}
$ cmake --build xo-ratio/${BUILDDIR}
$ cmake --install xo-ratio/${BUILDDIR}
```

### build with unit test coverage
```
$ cd xo-ratio
$ mkdir .build-ccov
$ cmake -DCMAKE_BUILD_TYPE=coverage -B .build-ccov
$ cmake --build .build-ccov
```

run coverage-enabled unit tests
```
$ cmake --build .build-ccov -- test
```

generate html+text coverage report
```
$ cmake --build .build-ccov -- ccov
```

browse to `.build-ccov/ccov/html/index.html`

### LSP support
```
$ cd xo-ratio
$ ln -s .build/compile_commands.json
```
