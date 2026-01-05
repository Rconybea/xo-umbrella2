# xo-facet


## Codegen

To scaffold a new facet Foo we need several c++ classes

* `AFoo`      abstract FOMO interface, with opaque data pointer
* `IFoo_Any`  variant placeholder, all methods terminate
* `IFoo_Xfer` template, delegates methods to a separate implementation class
* `RFoo`      template, inherits from a 2x-wide fat pointer (iface+data).

These classes all have parallel methods
Can generate these as follows:

1. write `xo-foo/idl/Foo.json5`.

   Supply attributes:
   - mode :: string
   - includes :: [string]
   - namespace1 :: string
   - namespace2 :: string
   - facet :: string
   - detail_subdir :: string
   - brief :: string
   - using_doxygen :: bool
   - doc :: [string]
   - types :: [{name, doc, definition}]
   - const_methods :: [{name, doc, return_type, args, const, noexcept, attributes}]
   - nonconst_methods :: [{name, doc, return_type, args, const, noexcept, attributes}]

Example in xo-object2/idl/Sequence.json5

2. generate:

```
$ cd xo-foo
$ ../xo-facet/codegen/genfacet.py --input ./idl/Foo.json5 --output include/xo/foo2
```

Alternatively in `xo-foo/CMakeLists.txt`

```
xo_add_genfacet(
    TARGET xo-foo2-facet-foo
    FACET Foo
    INPUT idl/Foo.json5
    OUTPUT_HTTP_DIR include/xo/foo2
    OUTPUT_IMPL_SUBDIR detail
    OUTPUT_CPP_DIR src/foo2)
```

then generate with:
```
cmake --build path/to/build -- xo-foo2-facet-foo
```
