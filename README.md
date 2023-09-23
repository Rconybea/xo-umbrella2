# intrusive reference counting

Refcnt is a small shared library supplying intrusive reference counting.

## Features

- base class `ref::Refcounted`.
  Application classes opt-in to reference counting by inheriting this class.
- common base simplifies connecting to common-base-object applications such as python, java etc.

## Getting Started

### build + install `indentlog` dependency

see [github/rconybea/indentlog](https://github.com/Rconybea/indentlog)

### copy `refcnt` repository locally
```
$ git clone git@github.com:rconybea/refcnt.git
$ ls -d refcnt
refcnt
```

### build + install
```
$ cd refcnt
$ mkdir build
$ cd build
$ cmake -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```

alternatively,  if you're a nix user:
```
$ git clone git@github.com:rconybea/xo-nix.git
$ ls -d xo-nix
xo-nix
$ cd xo-nix
$ nix-build -A refcnt
```

## Examples

### 1
```
#include "refcnt/Refcounted.hpp"

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
