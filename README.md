# unit library

Provides compile-time dimension checking and scaling.

Similar to `boost::units`,  but:
1. streamlined: assumes modern (c++20) support
2. supports fractional dimensions (rational powers)

## Documentation

- xo-unit documentation [under construction]: [documentation](https://rconybea.github.io/web/xo-unit/html/index.html)
- unit test coverage here: [coverage](https://rconybea.github.io/web/xo-unit/ccov/html/index.html)

## Getting Started

### build + install dependencies

- [github/Rconybea/reflect](https://github.com/Rconybea/reflect)

### build + install
```
$ cd xo-unit
$ mkdir .build
$ cd .build
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${PREFIX} -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make
$ make install
```

### build documentation
```
$ cd xo-unit
$ cmake --build .build -- sphinx
```
When this completes,  point local browser to `xo-unit/.build/docs/sphinx/index.html`.

### build for unit test coverage
```
$ cd xo-unit
$ mkdir .build-ccov
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_PREFIX_PATH=${PREFIX} -B .build-ccov
```

run coverage-enabled unit tests
```
$ cmake --build .build-ccov -- test
```

generate html+text coverage report
```
$cmake --build .build-ccov -- ccov
```

### LSP support
```
$ cd xo-unit
$ ln -s .build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
