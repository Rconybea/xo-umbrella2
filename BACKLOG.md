# reader2

## boolean operators

^ (xor), | (or), & (and), ~ (not)

# gc

## add_virtual_root

Explicit alternative to `add_gc_root_poly()` for virtual roots.
Since gc doesn't touch data pointer for virtual roots, we can pass
`obj<AGCObject>` instead of `obj<AGCObject>*`.

Will evolve some awkwardness in VSM: having to construct a purposeless
`obj<AGCObject,DVirtualSchematikaMachine>` just to appease the `add_gc_root_poly` signature.

## remove_root

Remove root doesn't work.
Reimplement root vector using DArenaVector or perhaps doubly-linked list.

## mutation_log

Need to implement! Copy from original gc

## facets

Collector facet should use json5 idl for consistency
