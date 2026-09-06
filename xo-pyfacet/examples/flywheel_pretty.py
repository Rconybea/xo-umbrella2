#!/usr/bin/env python3
"""Render an AllocFlywheel through a PrettySink, from python.

Shows the whole loop: python owns the arena configuration, the flywheel and
the sink, and drives a c++ render between two objects it holds.  Nothing here
is static or global -- everything is reclaimed by python's own refcounting
when the last reference drops.

Run (from the umbrella build):

    PYTHONPATH=.build/xo-pyindentlog2/src/pyindentlog2:.build/xo-pyfacet/src/pyfacet \
        python3 xo-pyfacet/examples/flywheel_pretty.py
"""

import xo_pyindentlog2 as il
import xo_pyfacet as f


def make_flywheel():
    """An AllocFlywheel: one primary arena plus two root sets.

    make_app() is a named factory, mirroring c++ -- an AllocFlywheel is
    heap-allocated and cannot be constructed any other way.  It is refcounted
    (it inherits Displayable -> Refcount), so python holds it through rp<>.
    """
    return f.AllocFlywheel.make_app(
        il.ArenaConfig(name="flywheel-storage", size=256 * 1024),
        il.ArenaConfig(name="flywheel-strong",  size=64 * 1024),
        il.ArenaConfig(name="flywheel-weak",    size=64 * 1024))


def render_to_string(obj, margin=60):
    """Render obj through a fresh accumulating sink; return the text.

    make2str builds a sink with no drain destination, so output() hands back
    everything written so far.  It is a copy -- the c++ output() returns a
    string_view that is only valid until the next write.
    """
    sink = il.PrettySink.make2str(il.PpConfig.scratch_plain(margin))
    obj.pretty(sink)
    return sink.output()


def render_to_stdout(obj, margin=60):
    """Render obj to stdout.  complete() ends the record and drains it."""
    sink = il.PrettySink.make2cout(il.PpConfig.scratch_plain(margin))
    obj.pretty(sink)
    sink.complete()


def main():
    fw = make_flywheel()

    print("1. render to a string")
    print("   ", repr(render_to_string(fw)))

    print("2. render to stdout")
    print("   ", end="", flush=True)
    render_to_stdout(fw)

    print("3. configuration is inspectable")
    cfg = il.PpConfig.scratch_plain(40)
    print("    cfg           =", cfg)
    print("    layout margin =", cfg.layout.soft_right_margin)
    print("    logbuf arena  =", cfg.logbuf.logbuf_config)

    print("4. configs are values -- with_* copies, it does not mutate")
    wide = cfg.with_soft_right_margin(120)
    print("    original      =", cfg.layout.soft_right_margin)
    print("    derived       =", wide.layout.soft_right_margin)

    print("5. the sink can be aimed at a named arena of your choosing")
    named = (il.PpConfig.scratch_plain(60)
             .with_logbuf_config(il.ArenaConfig(name="from-python",
                                                size=128 * 1024)))
    sink = il.PrettySink.make2str(named)
    fw.pretty(sink)
    print("    arena name    =", named.logbuf.logbuf_config.name)
    print("    output        =", repr(sink.output()))

    print("6. sinks are independent, and reusable across renders")
    a = il.PrettySink.make2str(il.PpConfig.scratch_plain(60))
    b = il.PrettySink.make2str(il.PpConfig.scratch_plain(60))
    fw.pretty(a)
    fw.pretty(a)          # accumulates
    fw.pretty(b)
    print("    a (rendered twice) =", repr(a.output()))
    print("    b (rendered once)  =", repr(b.output()))

    print("7. lifetimes are python's")
    del fw                # flywheel released here; sinks are unaffected
    import gc
    gc.collect()
    print("    flywheel released; a still holds", repr(a.output()))


if __name__ == "__main__":
    main()
