# flatstring library

Fixed-length no-allocation string implementation.

Features:
- char array representation with maximum size set at compile time.
- compile time construction from char array and string concatenation
- pointer-free implementation, instances can be used as template arguments
- To the extent practical, provides the same api as `std::string`: includes iterators,
  access methods, assignment, conversion operators.

Limitations:
- requires c++20
- not resizable.
- does not support wide characters.
- (asof April 2024) missing features: `insert`, `erase`, `push_back`, `append`, `replace`,
  `find`, `compare`, `starts_with`, `ends_with`, `contains`, `substr`.

## Documentation

- xo-flatstring documentation here: [documentation](https://rconybea.github.io/web/xo-flatstring/html/index.html)
- unit test coverage here: [coverage](https://rconybea.github.io/web/xo-flatstring/ccov/html/index.html)

## Getting started

### Install dependencies

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake) cmake macros
- [github/Rconybea/xo-indentlog](https://github.com/Rconybea/indentlog) logging (used by unit tests)

### Clone xo-flatstring

```
$ cd ~/proj  # for example
$ git clone https://github.com/rconybea/xo-flatstring
```

### build + install
```
$ cd xo-flatstring
$ mkdir .build
$ PREFIX=/usr/local   # for example
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -B .build
$ cmake --build .build
$ cmake --install .build
```

### build documentation
```
$ cd xo-flatstring
$ cmake --build .build -- docs
```
When complete, point local browser to `xo-flatstring/.build/docs/sphinx/index.html`

### build with test coverage
```
$ cd xo-flatstring
$ mkdir .build-ccov
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=coverage -B .build-ccov
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
$ cd xo-flatstring
$ ln -s .build/compile_commands.json
```
