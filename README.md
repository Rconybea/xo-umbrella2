# webutil library (header-only)

# dependencies

- xo-cmake [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

# clone repo
```
$ git clone git@github.com:Rconybea/xo-webutil.git
```

# build and install
```
$ cd xo-webutil
$ BUILDDIR=build     # for example
$ mkdir $BUILDDIR
$ cd $BUILDDIR
$ PREFIX=/usr/local  # for example
$ cmake -DCMAKE_MODULE_PATH=${PREFIX}/share/cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} ..
$ make
$ make install
```

# LSP support
```
$ cd xo-webutil
$ ln -s $BUILDDIR/compile_commands.json
```
