# random number generators

# to build + install locally

```
$ cd randomgen
$ mkdir build
$ cd build
$ cmake -DCMAKE_PREFIX_PATH=$(HOME)/local ..
$ make
$ make install
```

# to build + install to /usr/local (deprecated)

same as above,  but set `CMAKE_PREFIX_PATH` to `/usr/local`
