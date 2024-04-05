.. _unit-quantities:

.. toctree
   :maxdepth: 2

Unit Quantities
===============

Xo-unit uses the type system to represent units.
This is great for eliminating runtime overhead.

One place where we face some awkwardness is conversions involving multiple dimensions.
We'd like to write something concise like

.. code-block:: cpp

    meter / (second * second);

The difficulty arises because xo-unit represents `meter` and `second` by types
(``xo::unit::units::meter`` and ``xo::unit::units::second``); operators like `*` and `/`
apply to *values*, not types.

We'll present various ways to express rescaling below

Converting units
----------------

First, xo-unit provides constexpr unit quantities in namespace ``xo::unit::unit_qty``:

.. code-block:: cpp
   :linenos:

    static constexpr auto meter = qty::meters(1);
    static constexpr auto kilometer = qty::kilometers(1);
    // etc

Second,  a method ``quantity::with_units_from`` that takes units (only) from its argument:
``quantity::with_units_from`` just extracts its argument's unit_type to call ``quantity::with_units``.

.. code-block:: cpp
   :linenos:

    template <typename Unit, typename Repr>
    template <Quantity>
    auto quantity<Unit, Repr>::with_units_from(Quantity q) {
        return this->with_units<typename Quantity::unit_type>();
    }

Motivation is that it's easier to express an argument to `with_units_from`
than to express template arguments to `with_units`.

Prefer

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 5

    namespace u = xo::unit::unit_qty; // u::meter is a value
    namespace qty = xo::unit::qty;

    auto q1 = qty::kilometers(150.0) / qty::hours(0.5);
    auto q2 = q1.with_units_from(u::meter / u::second);

instead of the more verbose:

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 5-6

    namespace u = xo::unit::units;  // u::meter is a type

    auto q1 = qty::kilometers(150.0) / qty::hours(0.5);

    auto q2 = q1.with_units<unit_cartesian_product_t<u::meter,
                                                     unit_invert_t<u::second>>>();

Using basis units
-----------------

An alternative way to request multidimensional unit conversion is with basis units

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 4-5

    namespace u = xo::unit::units;  // u::meter is a type

    auto q1 = qty::kilometers(150.0) / qty::hours(0.5);
    auto q2 = q1.with_basis_unit<u::second>(); // q2 in km.s^-1
    auto q3 = q2.with_basis_unit<u::meter>();  // q3 in m.s^-1

With this technique we don't have to supply the basis dimension's exponent.
Instead we're just giving scale.
