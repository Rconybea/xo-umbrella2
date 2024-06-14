# intrusive reference counting

Refcnt is a small shared library supplying intrusive reference counting.

## Features

- base class `ref::Refcounted`.
  Application classes opt-in to reference counting by inheriting this class.
- common base simplifies connecting to common-base-object applications such as python, java etc.

## Getting Started

### build + install `xo-cmake` dependency (cmake macros)

see [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant for XO projects such as this one.

### build + install XO deps
```
$ xo-build --clone --configure --build --install xo-indentlog
```

### copy `refcnt` repository locally
```
$ xo-build --clone xo-refcnt
```

or equivalently
```
$ git clone git@github.com:rconybea/refcnt.git xo-refcnt
```

### build + install
```
$ xo-build --configure --build --install xo-refcnt
```

or equivalently:
```
$ mkdir xo-refcnt/.build
$ cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -S xo-refcnt -B xo-refcnt/.build
$ cmake --build xo-refcnt/.build
$ cmake --install xo-refcnt/.build
```

`CMAKE_PREFIX_PATH` should point to the prefix where `xo-indentlog` is installed

alternatively,  if you're a nix user:
```
$ git clone git@github.com:rconybea/xo-nix.git
$ ls -d xo-nix
xo-nix
$ cd xo-nix
$ nix-build -A xo-refcnt
```

### build for unit test coverage
```
$ cmake -DCMAKE_BUILD_TYPE=coverage -DCMAKE_PREFIX_PATH=${PREFIX} -S xo-refcnt -B xo-refcnt/.build-ccov
$ cmake --build xo-refcnt/.build-ccov
```

### LSP support
```
$ cd xo-refcnt
$ ln -s build/compile_commands.json  # lsp will look for compile_commands.json in the root of the source tree
```

## Examples

### 1
```
#include "xo/refcnt/Refcounted.hpp"

using xo::ref::Refcounted;

struct MyObject : public Refcounted {
     static rp<MyObject> make() { return new MyObject(); }

private:
     // intrusively-reference-counted objects should only be heap-allocated
     MyObject() { ... }
};

int main() {
     // create reference-counted instance
     auto x = MyObject::make();
     auto y = x;
     // x,y refer to the same instance.
     x = nullptr;
     // y holds last reference
     y = nullptr;
     // MyObject has been deleted
}
```

### 2

To log reference-counting activity

```
xo::ref::intrusive_ptr_set_debug(true);
```
