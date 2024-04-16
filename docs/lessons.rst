.. _lessons:

.. toctree
   :maxdepth: 2

Lessons
=======

This is a rogue's gallery of experiments,  typically unsuccessful.
One hurdle we've created for ourselves, is we need both gcc and clang to agree
that an expression can be computed at compile-time;
otherwise will get false alarms in our IDE (raised by LSP running in the background, which relies on clang).

Must Fully Initialize Memory
----------------------------

Struggled for a while with the implementation of :ref:xo::flatstring_concat

.. code-block:: cpp

    template <std::size_t N>
    flatstring::flatstring<N>() {
        if (N > 0)
            value_[0] = '\0';
    }


This implementation satisfies gcc, but not clang: in the following snippet, clang doesn't recognize ``tmp`` as constexpr:

.. code-block:: cpp

    constexpr n = ...;
    flatstring<n> tmp;

    static_assert(tmp.size() == ...);    // tmp not constexpr!

Correction is to prove to clang that every memory address owned by an empty ``flatstring`` is initialized:

.. code-block:: cpp

    template <std::size_t N>
    flatstring::flatstring<N>() {
        std::fill_n(value_, N, '\0');
    }
