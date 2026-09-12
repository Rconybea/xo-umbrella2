# xo-reflectable2

The `AReflectable` facet -- reflection for facet-object-model (fomo) data
types, so c++ code can interrogate a fomo object at runtime without knowing its
representation.  First consumer is `xo-printjson`.

**Scaffold.** `idl/Reflectable.json5` declares no methods, so `AReflectable` is
empty and nothing implements it yet.  The method it will carry is `self_tp()`,
returning a `xo::reflect::TaggedPtr` for the object's concrete representation --
the fomo counterpart to `xo::reflect::SelfTagging::self_tp()`.  Design and
remaining tickets: `.xo-backlog/reflectable2/`.

`xo-reflect` is deliberately NOT modified by any of this.  `TypeDescrExtra` and
`EstablishTdx` are public extension points, so the fomo-aware pieces can be
supplied from here instead.  That matters because reflect has far more
dependents than facet does, and hosting them in reflect would hand every one of
them a dependency on xo-facet:

```bash
for s in $(xo-deps --users-of=xo-reflect --format=names -q); do
    xo-deps --why=$s:xo-facet -q >/dev/null || echo "$s"
done
```

It is scaffolded rather than left as a bare directory so that `xo-build --all`
covers every entry in `xo-cmake/etc/xo/subsystem-list`; a subsystem listed there
but carrying no `CMakeLists.txt` makes `--all` fail at the configure step.

Namespace is `xo::reflect` -- shared with xo-reflect rather than separate, since
the facet is part of reflection and its method sits beside `SelfTagging`'s.
xo-ppsink and xo-indentlog2 share `xo::pp` the same way.

To add the method: edit `idl/Reflectable.json5`, then rebuild --
`xo_add_genfacet` regenerates `include/xo/reflectable2/detail/` and
`src/reflectable2/IReflectable_Any.cpp`, both of which are checked in.
