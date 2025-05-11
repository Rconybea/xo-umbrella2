# stochastic process library

constructive, simulation-aware models for stochastic processes

## Getting Started

### build + install dependencies

build+install these first

- xo-simulator [github.com/Rconybea/xo-simulator](https://github.com/Rconybea/xo-simulator)
- randomgen [github.com/Rconybea/randomgen](https://github.com/Rconybea/randomgen)

# build + install

## build
```
$ cd xo-process
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```
(also see .github/workflows/main.yml)

## build for unit test coverage
```
$ cd xo-process
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

## development

### LSP support

LSP looks for compile commands in the root of the source tree;
cmake creates them in the root of its build directory.

```
$ cd xo-process
$ ln -s build/compile_commands.json
```

## display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-process/build
$ cmake -LAH
```
