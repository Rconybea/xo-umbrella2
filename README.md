# ratio library

Header-only, constexpr library providing exact representation for rational numbers.

Relative to `std::ratio`:
1. Uses `constexpr` instead of creating new types.
   This means it can be used seamlessly at runtime.
2. Supports a few more arithmetic operations,
   for example exponentiation to integer powers.
3. Provides concept support (with c++20)
4. Requires modern (c++17) support to achieve this

## Getting Started

### install dependencies

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake) cmake macros

### build + install
```
$ cd xo-ratio
$ PREFIX=/usr/local # for example
$ BUILDDIR=.build   # for example
$ make ${BUILDDIR}
$ cmake -DCMAKE_PREFIX_PATH=${PREFIX} -B ${BUILDDIR}
```
