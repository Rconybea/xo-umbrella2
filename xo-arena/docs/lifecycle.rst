.. _lifecycle:

Lifecycle
=========

An arena moves through four states.
``mmap`` reserves a range of virtual addresses, not yet backed by physical memory (that comes later).
``alloc()`` hands out available memory by bumping a free pointer.
When available memory is exhausted, it obtains more by committing pages starting from the remaining
uncommitted reserved range.
``clear()`` resets the free pointer, but keeps all committed memory.
``unmap()`` returns the entire reserved range to the kernel.

The animation below runs those four phases in order, on a 32 MB reservation.

What to watch for
-----------------

The upper strip is the page ``free_`` currently sits in, drawn at byte resolution.
This may be a (4K on linux) regular page, or a 2MB superpage.

The ribbon beneath it is the whole reservation, with the current page bracketed.

.. raw:: html

    <iframe src="_static/arena-lifecycle.html?embed=1"
            title="Animated lifecycle of an xo-arena allocator"
            loading="lazy"
            style="width:100%; height:1860px; border:1px solid #ccc; border-radius:3px"></iframe>
    <p style="font-size:90%; margin-top:.4em">
      <a href="_static/arena-lifecycle.html" target="_blank" rel="noopener">Open the
      animation in its own tab</a> for more room.
    </p>

See also :doc:`DArena-reference` for the same picture in one screenful of text, and
:doc:`implementation` for how the pieces fit together.
