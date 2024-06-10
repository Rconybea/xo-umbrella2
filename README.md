# unit library

Provides compile-time dimension checking and scaling.

Similar to `boost::units`,  but:
1. streamlined: assumes modern (c++20) support
2. supports fractional dimensions (rational powers)

## Documentation

- xo-unit documentation [under construction]: [documentation](https://rconybea.github.io/web/xo-unit/html/index.html)
- unit test coverage here: [coverage](https://rconybea.github.io/web/xo-unit/ccov/html/index.html)

## Example

```
#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"

namespace q = xo::qty::qty;
namespace u = xo::qty::u;

constexpr auto t = q::minutes(2);
constexpr auto d = q::kilometers(2.5);

constexpr auto t2 = t*t;   // unit will be min^-2
constexpr auto a = d / t2; // unit will be km.min^-2

// convert to m.s^-2
constexpr quantity<u::meter / (u::second * u::second)> a2 = a;

//constexpr quantity<u::meter> a3 = a;  // dimension mismatch, will not compile

// get dimensionless scale value
double x = a2.scale();
```

## Getting Started

See [full install instructions](https://rconybea.github.io/web/xo-unit/html/install.html) for other installation strategies.

### build + install upstream dependencies

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)
- [github/Rconybea/xo-flatstring](https://github.com/Rconybea/xo-flatstring)
- [github/Rconybea/xo-ratio](https://github.com/Rconybea/xo-ratio)

(Below assumes they're installed using some common value for `PREFIX`)

### fetch xo-unit
```
$ cd ~/proj
$ git clone https://github.com/rconybea/xo-unit
```

### build + install
```
$ cd xo-unit
$ PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S . -B .build
$ cmake --build .build -j
$ cmake --install .build
```

### build documentation
```
$ cd xo-unit
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX}  --build .build -- sphinx
```
When this completes,  can point local browser to `xo-unit/.build/docs/sphinx/index.html`.

### build for unit test coverage

Note that unit tests involve additional dependencies:
- [github/Rconybea/xo-indentlog](https://github.com/Rconybea/indentlog)
- [github/Rconybea/xo-randomgen](https://github.com/Rconybea/randomgen)

```
$ cd xo-unit
$ mkdir .build-ccov
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_INSTALL_PREFIX=${PREFIX} -DENABLE_TESTING=1 -B .build-ccov
```

run coverage-enabled unit tests:
```
$ cmake --build .build-ccov -- test
```

generate html+text coverage report:
```
$ .build-ccov/gen-ccov
```
To see coverage,  can point local browser to `xo-unit/.build-ccov/ccov/html/index.html`

### LSP support
```
$ cd xo-unit
$ ln -s .build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```
