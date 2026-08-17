.. _lifecycle:

Lifecycle
=========

An arena moves through four states, and each one is a single call.  ``mmap`` reserves a
range of addresses and backs none of it.  ``alloc()`` hands memory out by bumping a
pointer, committing more whenever the pointer would run past what is backed.  ``clear()``
takes the pointer back to the start without giving up the memory.  ``unmap()`` returns
the range to the kernel.

The animation below runs those four phases in order, on a 32 MB reservation.

What to watch for
-----------------

**Committed memory moves in jumps, allocation does not.**  ``free_`` creeps forward with
every allocation; ``limit_`` sits still until an allocation would cross it, then advances
by a whole superpage.  That is ``DArena::expand()`` rounding its target up to
``arena_align_z_`` — 2 MB when the arena is at least one superpage, and the 4 KB page size
otherwise.  Turn the superpages off and the same run climbs in 4 KB steps, so fine that
the frontier looks continuous: the same memory, 200 times as many ``mprotect`` calls.

**Overhead is a function of object size.**  Each allocation carries an 8-byte header and
is padded to 8-byte alignment.  Drag the object-size slider down and watch the overhead
counter climb from a rounding error to a seventh of everything the arena hands out.

**Clearing is not freeing.**  ``clear()`` sets ``free_ = lo_`` and leaves ``committed_z_``
and ``limit_`` untouched, so the physical memory stays with the process, ready to be
handed out again without another syscall.  Only ``unmap()`` gives it back.

The upper strip is the page ``free_`` currently sits in, drawn at byte resolution, so
individual allocations and their headers are visible.  The ribbon beneath it is the whole
reservation, with the current page bracketed.

.. raw:: html

    <iframe src="_static/arena-lifecycle.html?embed=1"
            title="Animated lifecycle of an xo-arena allocator"
            loading="lazy"
            style="width:100%; height:1860px; border:1px solid #ccc; border-radius:3px"></iframe>
    <p style="font-size:90%; margin-top:.4em">
      <a href="_static/arena-lifecycle.html" target="_blank" rel="noopener">Open the
      animation in its own tab</a> for more room.
    </p>

The allocation sizes are a model rather than a captured trace, but the arithmetic follows
the source: 8-byte alignment, an 8-byte header per allocation, and commit targets rounded
up to ``arena_align_z_``.

See also :doc:`DArena-reference` for the same picture in one screenful of text, and
:doc:`implementation` for how the pieces fit together.
