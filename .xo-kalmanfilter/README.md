# kalman filter library

linear kalman filter implementation.

## Getting Started

### build + install dependencies

build+install these first

- xo-reactor [github.com/Rconybea/xo-reactor](https://github.com/Rconybea/xo-reactor)

See `.github/workflows/main.yml` in this repo for example build+install on ubuntu

### build + install xo-kalmanfilter
```
$ cd xo-kalmanfilter
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
$ cd xo-kalmanfilter
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

## Development

### LSP support

LSP looks for compile commands in the root of the source tree;
cmake creates them in the root of its build directory.

```
$ cd xo-kalmanfilter
$ ln -s build/compile_commands.json
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-kalmanfilter/build
$ cmake -LAH
```
