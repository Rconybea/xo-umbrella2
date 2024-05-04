.. _examples:

.. toctree
   :maxdepth: 2

Examples
========

Compile-time unit inference
---------------------------

See ``xo-unit/examples/ex1`` for code below.

Units propagate through familiar arithmetic expressions:

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 15-16

    #include "xo/unit/quantity.hpp"
    #include "xo/unit/quantity_iostream.hpp"
    #include <iostream>

    int
    main () {
        namespace q = xo::qty::qty;
        namespace su = xo::qty::su;
        using xo::qty::quantity;
        using namespace std;

        constexpr auto t = q::minutes(2);
        constexpr auto d = q::kilometers(2.5);

        constexpr auto t2 = t*t;
        constexpr auto a = d / (t*t);

        cerr << "t: " << t << ", d: " << d
             << ", t^2: " << t2
             << ", d.t^-2: " << a
             << endl;
    }


with output:

.. code-block::

    t: 2min, d: 2.5km, t^2: 4min^2, d.t^-2: 0.625km.min^-2

Remarks:

- ``xo::qty::quantity`` performs unit consistency checking and propagation at compile time.  Runtime space/time overhead is zero.
- Unit abbreviations (such as ``kg.min^-2`` above) are computed at compile time.  See ``xo::flatstring`` for constexpr string implementation.
- Units are sticky: since we expressed ``t`` in minutes, ``(t*t)`` and ``d/(t*t)`` also use minutes.
- Unit ordering is sticky.  Distance appears on the left of time in printed value of ``d/(t*t)``
  because it was on the left-hand side of ``operator/``
- See ``xo::xquantity`` for parallel implementation that defers unit checking until runtime.

We can use static asserts to prove that units are being computed at compile-time

.. code-block:: cpp
   :linenos:

    static_assert(std::same_as<decltype(t)::repr_type, double>);
    static_assert(sizeof(t) == sizeof(double));
    static_assert(t.scale() == 2);
    static_assert(t.abbrev() == flatstring("min"));

    static_assert(std::same_as<decltype(d)::repr_type, double>);
    static_assert(sizeof(d) == sizeof(double));
    static_assert(d.scale() == 2.5);
    static_assert(d.abbrev() == flatstring("km"));

    static_assert(std::same_as<decltype(t2)::repr_type, double>);
    static_assert(sizeof(t2) == sizeof(double));
    static_assert(t2.scale() == 4);
    static_assert(t2.abbrev() == flatstring("min^2"));

    static_assert(std::same_as<decltype(a)::repr_type, double>);
    static_assert(sizeof(a) == sizeof(double));
    static_assert(a.scale() == 0.625);
    static_assert(a.abbrev() == flatstring("km.min^-2"));


Explicit scale conversion using rescale method
----------------------------------------------

Can convert between compatible units explictly:

See ``xo-unit/examples/ex2`` for code below.

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 9

    ...

    constexpr auto t = q::minutes(2);
    constexpr auto d = q::kilometers(2.5);

    constexpr auto t2 = t*t;
    constexpr auto a = d / (t*t);

    constexpr auto a2 = a.template rescale<su::meter / (su::second * su::second)>();

    static_assert(a2.abbrev() == flatstring("m.s^-2"));

    cerr << "a2: " << a2 << endl;

with output:

.. code-block:: cpp

    a2: 0.173611m.s^-2

Explicit scale conversion from another quantity
-----------------------------------------------

Alternatively,  can transfer units from another quantity

See ``xo-unit/example/ex2`` for code below

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 1-2

    constexpr auto au = q::meter / (q::second * q::second);  /* just for units */
    constexpr auto a3 = with_units_from(a, au);  /* a, but with units from au */

    static_assert(a3.abbrev() == flatstring("m.s^-2"));

    cerr << "a3: " << a3 << endl;

with output:

.. code-block:: cpp

    a3: 0.173611m.s^-2


Implicit Scale conversion triggered by assignment
-------------------------------------------------

Another way to convert units is to assign to a variable
with desired units -- this works because the units are encoded
as part of the assigned variable's type.

See ``xo-unit/example/ex3`` for code below

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 11-12

    int
    main () {
        namespace q = xo::qty::qty;
        namespace nu = xo::qty::nu;
        using xo::qty::with_units;
        using xo::qty::stdquantity;
        using xo::qty::quantity;
        using xo::flatstring;
        using namespace std;

        constexpr stdquantity<nu::second> t = q::minutes(2);
        constexpr stdquantity<nu::meter> d = q::kilometers(2.5);

        constexpr auto t2 = t*t;
        constexpr auto a = d / (t*t);

        cerr << "t: " << t << ", d: " << d
             << ", d.t^-2: " << a
             << endl;
    }

with output:

.. code-block::

    t: 120s, d: 2500m, d.t^-2: 0.17e611m.s^-2

Remarks:

*  Assignment to ``t`` converted to representation ``double``.
   We could have used :code:`stdquantity<int, u::second>` to preserve
   right-hand-side representation.

Scale conversion triggered by arithmetic
----------------------------------------

When representing a particular quantity,
xo-unit uses at most one scale for each :term:`basis dimension` associated with the unit for that quantity.
When an arithmetic operator encounters basis units involving two different scales,
the operator will adopt the scale provided by the left-hand argument:

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 11

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int main() {
        namespace u = xo::unit;
        namespace qty = xo::units::qty;
        using namespace std;

        auto t1 = qty::milliseconds(1);
        auto t2 = qty::minutes(1);
        auto p = t1 * t2;

        cerr << "t1: " << t1 << ", t2: " << t2 << ", p: " << p << endl;
    }

with output:

.. code-block::

    t1: 1ms, t2: 1min, t1*t2: 60000ms^2

Dimensionless quantities collapse automatically
-----------------------------------------------

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 14-15

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int main() {
        namespace u = xo::unit;
        namespace qty = xo::units::qty;
        using namespace std;

        auto t1 = qty::milliseconds(1);
        auto t2 = qty::minutes(1);
        auto r1 = t1 / t2.with_repr<double>();
        auto r2 = t2 / t1.with_repr<double>();

        static_assert<same_as<decltype(r1), double>);
        static_assert<same_as<decltype(r2), double>);

        cerr << "t1: " << t1 << ", t2: " << t2 << ", t1/t2: " << r1 << ", t2/t1: " << r2 << endl;
    }

with output:

.. code-block::

    t1: 1ms, t2: 1min, t1/t2: 1.66667e-05, t2/t1: 60000


Fractional dimension
--------------------

Fractional dimensions are supported;   they work in the same way as familiar integral dimensions.

Only caveat is that converting between fractional units with different scales creates a floating-point conversion factor,
which may incur loss of precision based on floating-point roundoff.

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 15

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int
    main () {
        namespace u = xo::unit::units;
        namespace qty = xo::unit::qty;
        using namespace std;

        /* 20% volatility over 250 days (approx number of trading days in one year) */
        auto q1 = qty::volatility250d(0.2);
        /* 10% volatility over 30 days */
        auto q2 = qty::volatility30d(0.1);

        static_assert(q2.basis_power<dim::time, double> == 0.5);

        auto sum = q1 + q2;
        auto prod = q1 * q2;

        static_assert(prod.basis_power<dim::time> == 1);

        cerr << "q1: " << q1 << ", q2: " << q2 << ", q1+q2: " << sum << ", q1*q2" << prod << endl;
    }

with output:

.. code-block::

    q1: 0.2yr250^-(1/2), q2: 0.1mo^-(1/2), q1+q2: 0.488675yr250^(1/2), q1*q2: 0.057735yr250^-1
