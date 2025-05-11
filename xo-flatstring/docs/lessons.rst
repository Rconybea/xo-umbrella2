.. _lessons:

.. toctree
   :maxdepth: 2

Lessons
=======

This is a rogue's gallery of experiments,  typically unsuccessful.
One hurdle we've created for ourselves, is we need both gcc and clang to agree
that an expression can be computed at compile-time;
otherwise will get false alarms in our IDE (raised by LSP running in the background, which relies on clang).

Must fully initialize memory
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

Still need equality comparison alongside spaceship operator
-----------------------------------------------------------

Had the impression that spaceship operator for :ref:xo::flatstring would be sufficient
to get all six comparison operators:

.. code-block:: cpp

    template <std::size_t N1,
              std::size_t N2>
    constexpr auto
    operator<=>(const flatstring<N1> & s1,
                const flatstring<N2> & s2) noexcept
    {
        return (std::string_view(s1) <=> std::string_view(s2));
    }

We observe this is not the case,  at least with gcc 13.1;  need to separately define :ref:xo::operator==

.. code-block:: cpp

    template <std::size_t N1,
              std::size_t N2>
    constexpr bool
    operator==(const flatstring<N1> & s1,
               const flatstring<N2> & s2) noexcept
    {
        return ((s1 <=> s2) == std::strong_ordering::equal);
    }

Constexpr strict about pointer arithmetic
-----------------------------------------

Initially attempted to implement :ref:xo::flatstring reverse iterators using char pointers.

Notice there's an assymetry between reverse iterators and forward iterators.
We can (and do) implement forward iterators using char pointers.
The natural value of ``flatstring::end()`` is a char pointer referring to just past the end of
the string,  i.e. to its null terminator.  From the compiler's perspective,  this is an ordinary
char pointer,  just like other iterator values.

For reverse iterators this isn't the case.  The natural value for ``flatstring::rend()`` might
seem to be a char pointer referring to just before the first character in the string.
However this is no longer a valid pointer address -- dereferencing would be undefined behavior.

In particular,  with this implementation, gcc demotes ``flatstring::rend()`` to non-constexpr

Workaround is to implement a shim iterator class, where representation is pointer to the
character just after the one the iterator position; iterator's ``operator*`` adjusts pointer before
dereferencing.

This works because gcc can observe that we never dereference a reverse iterator with pointer value
at the beginning of a flatstring.
