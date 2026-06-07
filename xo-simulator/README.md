# simulator library

in-memory deterministic simulator

## Getting Started

### build + install dependencies

build+install these first

- xo-reactor [github.com/Rconybea/xo-reactor](https://github.com/Rconybea/xo-reactor)
- xo-ordinaltree [github.com/Rconybea/xo-ordinaltree](https://github.com/Rconybea/xo-ordinaltree)

### build + install xo-simulator
```
$ cd xo-simulator
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```
(also see .github/workflows/main.yml)

### build for unit test coverage
```
$ cd xo-simulator
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

## Development

### LSP support

LSP looks for compile commands in the root of the source tree;
cmake creates them in the root of its build directory.

```
$ cd xo-simulator
$ ln -s build/compile_commands.json
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-simulator/build
$ cmake -LAH
```
