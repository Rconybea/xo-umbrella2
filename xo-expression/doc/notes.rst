Random notes.

Type inference.

Expressions represent parsed abstract syntax trees.
We use the same data structure to represent syntax trees both before and after
type inference.

We record each Expression's type using a type_ref instance.
A type_ref does two things:
1. gives the expression's type a unique name
2. records expression's concrete type once type infrerence has completed.

Type inference resolves each Expression's type to a concrete TypeDescr.
An Expression tree can't be compiled unless all reachable Expression nodes
are resolved to concrete types, i.e. type inference has completed successfully.
