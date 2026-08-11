# xo-hashable2

The `AHashable` facet -- content hashing for facet-object-model data types.

**Placeholder.** The facet is scaffolded but not designed: `idl/Hashable.json5`
declares no methods, so `AHashable` is empty and nothing implements it yet.

It is scaffolded rather than left as a bare directory so that
`xo-build --all` covers every entry in `xo-cmake/etc/xo/subsystem-list`.  A
subsystem listed there but carrying no `CMakeLists.txt` makes `--all` fail at
the configure step, which is why the flag was unusable in this tree until
2026-08-10.

To design the facet: edit `idl/Hashable.json5` (methods, and `namespace2`, which
is still the scaffolded default), then rebuild -- `xo_add_genfacet` regenerates
`include/xo/hashable2/detail/` and `src/hashable2/IHashable_Any.cpp`, both of
which are checked in.
