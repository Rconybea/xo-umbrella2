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

## Getting started

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
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -B .build-ccov
$ cmake --build .build-ccov
```

run coverage-enabled unit tests
```
$ (cd .build-ccov && ctest)
```

generate html+text coverage report
```
$ .build-ccov/gen-ccov
```

browse to `.build-ccov/ccov/html/index.html`

### LSP support
```
$ cd xo-flatstring
$ ln -s .build/compile_commands.json
```
