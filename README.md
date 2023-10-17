# websock library

http library including websocket support.
Built around the C-library libwebsocket

# dependencies

build+install these first

- xo-somelib [github.com/Rconybea/xo-somelib]

# build + install

## build
```
$ cd websock
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
$ cd xo-websock
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake  -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

# development

## LSP support

LSP looks for compile commands in the root of the source tree;
cmake creates them in the root of its build directory.

```
$ cd xo-websock
$ ln -s build/compile_commands.json
```

## display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd websock/build
$ cmake -LAH
```
