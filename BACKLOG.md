# reader2

## globalsymtab for essential primitives

rework essential primitives (*,/,+,-,==,!=,..)
so parser doesn't rely on magic globalsymtab lookup strings.

## support missing primitives

tokenizer support + utest for !=,<,>,<=,>=

## boolean operators

^ (xor), | (or), & (and), ~ (not)

# gc

## remove_root

Remove root doesn't work.
Reimplement root vector using DArenaVector or perhaps doubly-linked list.

## mutation_log

Need to implement! Copy from original gc

## facets

Collector facet should use json5 idl for consistency
