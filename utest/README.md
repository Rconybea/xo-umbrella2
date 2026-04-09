Unit test here relies on generated facet support.

Also rehearsing subsystem setup that we use in other xo subsystems
to perform facet registration.
(This is for form's sake.  Could just as well invoke directly from gc_utest_main.cpp).

idl/                    config for obj<AGCObjectVisitor,DMockCollector>.


IGCObject_DMockCollector.*pp  automatically generated from idl/

MockCollector.hpp       fop header file for DX1Collector.

DMockCollector.*pp      mock ACollector impl. Wraps a GCObjectStore pointer.
                        Load-bearing for GCObjectStore.test.cpp

init_gc_utest.*pp       mock subsystem setup for this directory,
                        as if it were an "xo-gc-utest" subsystem

GCObjectStore.test.cpp  Generative tests for GCObjectStore.
                        Generate non-trivial object graphs,
                        verify evacuation algorithm.

----------------------------------------------------------------
These don't use anything downstream of idl/

Collector.test.cpp      X1 tests. Does some random allocs,
                        but doesn't exercise GC features

X1Collector.test.cpp    standalone X1Collector test.
                        does some atomic allocs + verifies
                        GC behavior for a few xo-object2/ types.
                        Verify simple tests here.

DX1CollectorIterator.test.cpp  Collector iterator test
