# xo-facet documentation master file

xo-facet documentation
======================

xo-facet provides an object model that supports runtime polymorphism with interfaces and data kept separate.
Design operates on similar lines to rust traits, haskell type clases, and go interfaces.

Principles
----------

* Keep interfaces and data structures separate.
  An object is represented using a combination of exactly two pointers:
  an interface pointer and a data pointer.

* An interface pointer implements an abstract facet.
  A facet has only abstract methods, and no state.

* An interface pointer is analogous to a vtable pointer in a regular
  c++ object.  It identifies a suite of related functions that operate
  on a particular data type.

* A data pointer is like a pointer to a c struct.
  Data objects are passive, except for necessary ctors/dtors.
  Runtime polymorphism works seamlessly across different data types
  without requiring any prearrangement such as sharing a common
  base class.

* We make 'familiar c++ objects', on demand, by pairing an interface pointer
  with a data pointer. Unlike usual c++ practice, we expect such objects
  to be transient. To represent persistent state, we rely
  solely on data pointers.

* Since interface+data are separate,
  we can easily swap out one interface for another.

This gives us several benefits:

* A data type can easily particpate in polymorphism across different facets,
  without complicating object representation. To convert an object to use a
  different facet, we just swap out the interface pointer.

* Interface and data pointers can arrive at the doorstep of a computation
  by different pathways. Often the pathway for an interface pointer is
  simpler that the pathway for a data pointer. This increases scope for
  devirtualization.


.. toctree::
   :maxdepth: 2
   :caption: xo-facet contents

   implementation
   glossary
   genindex
   search
