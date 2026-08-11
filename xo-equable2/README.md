# xo-equable2

The `AEquable` facet -- equality comparison for facet-object-model data types.

**Placeholder.** The facet is scaffolded but not designed: `idl/Equable.json5`
declares no methods, so `AEquable` is empty and nothing implements it yet.

It is scaffolded rather than left as a bare directory so that
`xo-build --all` covers every entry in `xo-cmake/etc/xo/subsystem-list`.  A
subsystem listed there but carrying no `CMakeLists.txt` makes `--all` fail at
the configure step, which is why the flag was unusable in this tree until
2026-08-10.

To design the facet: edit `idl/Equable.json5` (methods, and `namespace2`, which
is still the scaffolded default), then rebuild -- `xo_add_genfacet` regenerates
`include/xo/equable2/detail/` and `src/equable2/IEquable_Any.cpp`, both of which
are checked in.
