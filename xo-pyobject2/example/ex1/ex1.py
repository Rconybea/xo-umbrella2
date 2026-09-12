#!/usr/bin/env python3
"""Typical xo-pyobject2 setup, from configuration to boxed objects.

Walks the path a python program takes to allocate xo objects: bring up the
subsystem contexts, make a flywheel to allocate from, box some values, render
them, and see where the memory went.

The emphasis here is what xo-pyobject2 adds -- objects, the handles that keep
them alive, and how their memory is reported.  Sink configuration (PpConfig,
with_* copying, aiming a sink at a named arena) is covered thoroughly by
xo-pyfacet/examples/flywheel_pretty.py, so it is only used here, not explored.

Run (from the umbrella build):

    .build/xo-python xo-pyobject2/example/ex1/ex1.py

or from a standalone subsystem build:

    xo-pyobject2/.build/xo-python xo-pyobject2/example/ex1/ex1.py
"""

import gc

import xo_pyarena as mm
import xo_pyindentlog2 as il
import xo_pyfacet as f
import xo_pyobject2 as o


def main():
    print("1. configure")
    # configure_all() brings up xo-indentlog2 and xo-facet together, each with
    # its make_default().  The long form is visible in flywheel_pretty.py:
    #
    #     il_cx = il.configure(il.Indentlog2Config(il.PpConfig.plain(), 64*1024))
    #     facet_cx = f.configure(f.FacetConfig(1024, 1024), il_cx)
    #
    # Configuration is ONE-SHOT per process: a second configure() raises, and
    # there is no reset.  Importing a module only registers types; nothing is
    # established until you ask for it here.
    cx = f.configure_all()
    print("    facet context     =", repr(cx))
    print("    indentlog2 context=", repr(cx.indentlog2_appcx()))

    print("2. a flywheel to allocate from")
    # An AllocFlywheel is a primary arena plus strong and weak root sets.
    # make_default_app() picks capacities; make_app() is the form that names
    # its arenas, which is what makes the memory report below legible:
    #
    #     f.AllocFlywheel.make_app(
    #         cx,
    #         mm.ArenaConfig(name="store",  size=256*1024),
    #         mm.ArenaConfig(name="strong", size=64*1024),
    #         mm.ArenaConfig(name="weak",   size=64*1024))
    fw = f.AllocFlywheel.make_default_app(cx)
    print("    flywheel          =", repr(fw))

    print("3. box some values")
    # Float.make() is a named factory, not a constructor: a DFloat lives in an
    # arena, so it cannot be made without being told which one.  What comes
    # back is a HANDLE into that arena, pinned by a strong root -- not a python
    # object holding a copy of the double.
    xs = [o.Float.make(fw, v) for v in (1.5, 2.25, 3.125)]
    print("    values            =", [x.value() for x in xs])

    print("4. rendering")
    # repr() goes through the configured pretty-printer, so python shows the
    # text c++ does.
    print("    repr(xs[0])       =", repr(xs[0]))

    # A consequence worth knowing: once a context is configured, that printer
    # wraps at the configured margin, so a repr of anything structured may be
    # multi-line.  That is deliberate -- it is what lets repr be tuned rather
    # than hardwired to one line -- but it does surprise.
    print("    repr(an ArenaConfig), wrapped by the pretty-printer:")
    for line in repr(mm.ArenaConfig(name="demo", size=1 << 18)).splitlines():
        print("       ", line)

    # pretty() renders into a sink you supply.  make2str accumulates, so
    # output() hands back everything written to it so far.
    sink = il.PrettySink.make2str(il.PpConfig.scratch_plain(60))
    for x in xs:
        x.pretty(sink)
    print("    three, one sink   =", repr(sink.output()))

    print("5. where the memory went")
    # Each context reports only the pools it owns, so walking the chain and
    # concatenating cannot double-count.  The flywheel's three are its arena
    # and its two root sets; the facet context's are its registries; the
    # indentlog2 context's are per-thread and appear once this thread renders.
    for label, pools in (("flywheel  ", fw.visit_pools()),
                         ("facet cx  ", cx.visit_pools()),
                         ("indentlog2", cx.indentlog2_appcx().visit_pools())):
        print("    {} -> {}".format(label, [p.name for p in pools]))

    store = fw.visit_pools()[0]
    print("    store: used={} committed={} reserved={}"
          .format(store.used, store.committed, store.reserved))

    print("6. lifetimes")
    # The handle owns a reference to the flywheel, so the arena outlives every
    # other reference to it.  This is the part that differs from a plain
    # pointer: dropping the flywheel does not strand the objects in it.
    keep = xs[0]
    del fw, xs
    gc.collect()
    print("    flywheel dropped; handle still reads", keep.value())


if __name__ == "__main__":
    main()
