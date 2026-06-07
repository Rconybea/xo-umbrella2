# reactor library

in-memory queuing system

## Getting Started

### build + install dependencies

build+install these first

- xo-reflect [github.com/Rconybea/xo-reflect](https://github.com/Rconybea/reflect)
- xo-callback [github.com/Rconybea/xo-callback](https://github.com/Rconybea/xo-callback)
- xo-webutil [github.com/Rconybea/xo-webutil](https://github.com/Rconybea/xo-webutil)

### build + install xo-reactor
```
$ cd xo-reactor
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```
(also see .github/workflows/main.yml)

## Development

### build for unit test coverage
```
$ cd xo-reactor
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

### LSP support

LSP looks for compile commands in the root of the source tree;
cmake creates them in the root of its build directory.

```
$ cd xo-reactor
$ ln -s build/compile_commands.json
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-reactor/build
$ cmake -LAH
```
