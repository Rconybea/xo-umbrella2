# reactor library

In-memory queuing system.

## Getting Started

`xo-reactor2` is part of the XO project.
To install all of XO, see [github/Rconybea/xo-umbrella2](https://github.com/rconybea/xo-umbrella2).

To install `xo-reactor2` independently, will also need its XO dependencies

### Build + Install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients, along with build assistant `xo-build` for XO projects like this one.

### Build + Install other XO dependencies 

To see commands without running them, use `-n`:
```
$ xo-build --with-deps --clone -n xo-reactor2
```

Build and install in dependency order:
```
$ xo-build --with-deps --clone --configure --build --install xo-reactor2
```


