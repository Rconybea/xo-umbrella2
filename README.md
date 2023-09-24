# plugin initialization support

subsys is a small header-only library providing support for plugin initialization

## Features

- provide application control of initialization order across c++ libraries
- circumvents the 'static order initialization fiasco'
- ensure initialization code runs exactly once if subsystem is linked
- enforce initialization order constraints
- defend against static linker stripping essential initialization code
- designed to work cleanly for libraries integrating into existing executable like python, java runtime, ..
- initialization state browseable at runtime

## Getting Started

### build + install `indentlog` dependency

see [github/rconybea/indentlog](https://github.com/Rconybea/indentlog)

### copy `subsys` repository locally
```
$ git clone git@github.com:rconybea/subsys.git
$ ls -d subsys
subsys
```

### build + install
```
$ cd subsys
$ mkdir build
$ cd build
$ cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```

`CMAKE_PREFIX_PATH` should point to prefix where `indentlog` is installed

alternatively,  if you're a nix user:
```
$ git clone git@github.com:rconybea/xo-nix.git
$ ls -d xo-nix
xo-nix
$ cd xo-nix
$ nix-build -A subsys
```

### build for unit test coverage
```
$ cd subsys
$ mkdir ccov
$ cd ccov
$ cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
```

## Examples

### 1
```
// initialization code in .hpp for a subsystem foo,  that relies on related subsystem bar

#include "subsys/Subsystem.hpp"

enum S_foo_tag {};  /* tag to represent initialization of subsystem foo */

template<>
struct InitSubsys<S_foo_tag> {
     static void init() {
         // plugin initialization for subsystem foo
     }

    static InitEvidence require() {
          InitEvidence retval;

          // demand initialization of dependent subsystem  bar,
          // before initialization subsystem foo
          //
          retval ^= InitSubsys<S_bar_tag>::require();

          // initialization of this subsystem foo
          retval ^= Subsystem::provide<S_foo_tag>("foo", &init);

          return retval;
    }
};
```

```
// in application code that relies on foo (perhaps along with other subsystems),
// for example in a pybind11 module:
//
PYBIND11_MODULE(pyfoo, m) {
    // include foo in initialization set
    InitSubsys<S_foo_tag>::require();
    // ensure foo + dependencies are initialized
    Subsystem::initialize_all();

    ...
}
```
