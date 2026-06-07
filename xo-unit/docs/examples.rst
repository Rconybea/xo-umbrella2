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
   :emphasize-lines: 14-15

    #include "xo/unit/quantity.hpp"
    #include "xo/unit/quantity_iostream.hpp"
    #include <iostream>

    int
    main () {
        namespace q = xo::qty::qty;
        namespace su = xo::qty::su;
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

We can use static asserts to prove that units are being computed at compile-time

.. code-block:: cpp
   :linenos:

    static_assert(std::same_as<decltype(t)::repr_type, int>);
    static_assert(sizeof(t) == sizeof(double));
    static_assert(t.scale() == 2);
    static_assert(t.abbrev() == flatstring("min"));

    static_assert(std::same_as<decltype(d)::repr_type, double>);
    static_assert(sizeof(d) == sizeof(double));
    static_assert(d.scale() == 2.5);
    static_assert(d.abbrev() == flatstring("km"));

    static_assert(std::same_as<decltype(t2)::repr_type, int>);
    static_assert(sizeof(t2) == sizeof(double));
    static_assert(t2.scale() == 4);
    static_assert(t2.abbrev() == flatstring("min^2"));

    static_assert(std::same_as<decltype(a)::repr_type, double>);
    static_assert(sizeof(a) == sizeof(double));
    static_assert(a.scale() == 0.625);
    static_assert(a.abbrev() == flatstring("km.min^-2"));

Remarks:

- ``xo::qty::quantity`` performs unit consistency checking and propagation at compile time.  Runtime space/time overhead is zero.
- Units are sticky: since we expressed ``t`` in minutes, ``(t*t)`` and ``d/(t*t)`` also use minutes.
- Unit ordering is sticky.  Distance appears on the left of time in printed value of ``d/(t*t)``
  because it was on the left-hand side of ``operator/``
- ``xo-unit`` copies representation from the argument to factory functions ``q::minutes``, ``q::kilometers`` etc.
- Binary operators take representation from the 'most precise' argument,  as prescribed by ``std::common_type_t``.
- Unit abbreviations (such as ``kg.min^-2`` above) are computed at compile time.
  See ``xo::flatstring`` for constexpr string implementation.
- See ``xo::xquantity`` for parallel implementation that defers unit checking until runtime.


Explicit scale conversion
-------------------------

Can convert between compatible units explictly,
using:

1. ``xo::qty::with_units`` (template function)
2. ``quantity.rescale_ext`` (template method)
3. ``xo::qty::with_units_from`` (template function)

See ``xo-unit/examples/ex2`` for code below.

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 10,13,16-17

    #include "xo/unit/quantity.hpp"
    #include "xo/unit/quantity_iostream.hpp"
    #include <iostream>

    int
    main () {
        namespace q = xo::qty::qty;
        namespace u = xo::qty::u;
        using xo::qty::with_units_from;
        using xo::qty::with_units;
        using xo::qty::quantity;
        using xo::flatstring;
        using namespace std;

        constexpr auto t = q::minutes(2);
        constexpr auto d = q::kilometers(2.5);

        constexpr auto t2 = t*t;
        constexpr auto a = d / (t*t);

        cerr << "t: " << t << ", d: " << d
             << ", t^2: " << t2
             << ", d.t^-2: " << a
             << endl;

        constexpr auto a2 = with_units<u::meter / (u::second * u::second)>(a);

        static_assert(a2.abbrev() == flatstring("m.s^-2"));

        cerr << "a2: " << a2 << endl;

        constexpr auto a3 = a.rescale_ext<u::meter / (u::second * u::second)>();

        static_assert(a3.abbrev() == flatstring("m.s^-2"));

        cerr << "a3: " << a3 << endl;

        constexpr auto au = q::meter / (q::second * q::second);
        constexpr auto a4 = with_units_from(a, au);

        static_assert(a4.abbrev() == flatstring("m.s^-2"));

        cerr << "a4: " << a4 << endl;
    }

with output:

.. code-block:: cpp

    a2: 0.173611m.s^-2
    a3: 0.173611m.s^-2
    a4: 0.173611m.s^-2

Implicit Scale conversion
-------------------------

Another way to convert units is to assign to a variable
with desired units -- this works because the units are encoded
as part of the assigned variable's type.

See ``xo-unit/example/ex3`` for code below

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 12-13

    int
    main () {
        namespace q = xo::qty::qty;
        namespace u = xo::qty::u;
        using xo::qty::quantity;

        constexpr quantity<u::second> t = q::minutes(2);
        constexpr quantity<u::meter> d = q::kilometers(2.5);

        constexpr auto t2 = t*t;
        constexpr auto a = d / (t*t);

        std::cerr << "t: " << t << ", d: " << d
                  << ", d.t^-2: " << a
                  << std::endl;
    }

with output:

.. code-block::

    t: 120s, d: 2500m, d.t^-2: 0.17e611m.s^-2

Remarks:

- Assignment to ``t`` converted to representation ``double``.
  We could have instead used :code:`quantity<u::second, int>` to propagate
  right-hand-side representation

Scale conversion and arithmetic
-------------------------------

When representing a particular quantity,
xo-unit uses at most one scale for each :term:`basis dimension` associated with the unit for that quantity.
When an arithmetic operator encounters basis units involving two different scales,
the operator will adopt the scale provided by the left-hand argument:

See ``xo-unit/example/ex4`` for code below

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 11

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int main() {
        namespace q = xo::qty::qty;

        auto t1 = qty::milliseconds(1);
        auto t2 = qty::minutes(1);
        auto p = t1 * t2;

        std::cerr << "t1: " << t1 << ", t2: " << t2 << ", p: " << p << std::endl;
    }

with output:

.. code-block::

    t1: 1ms, t2: 1min, t1*t2: 60000ms^2


Dimensionless quantities unwrap implicitly
------------------------------------------

Conversely,  compiler rejects attempt to implictly unwrap a dimensioned quantity.

See ``xo-unit/examples/ex4`` for code below.

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 23,26

    #include "xo/unit/quantity.hpp"
    #include "xo/unit/quantity_iostream.hpp"
    #include <iostream>

    int
    main () {
        namespace q = xo::qty::qty;

        auto t1 = q::milliseconds(1);
        auto t2 = q::minutes(1);

        auto r1 = t1 / with_repr<double>(t2);

        static_assert(r1.is_dimensionless());
        static_assert(!t2.is_dimensionless());

        static_assert(std::same_as<static_cast<double>(r1), double>);

        // r1_value: assignment compiles,  since r1 dimensionless
        double r1_value = r1;

        // r2_value: bad assignment won't compile,  'cannot convert' error
        //double r2_value = t2;

        std::cerr << "t1: " << t1 << ", t2: " << t2 << ", t1/t2: " << r1_value << std::endl;
    }

with output:

.. code-block::

    t1: 1ms, t2: 1min, t1/t2: 1.66667e-05


Fractional dimension
--------------------

Fractional dimensions have limited support.
Prior to c++26 we can only support fractional dimensions with denominator 2,
such as powers -3/2, -1/2, +1/2, +3/2 etc.

c++26 will enable support for support fractional dimensions involving other ratios,
by offering constexpr ``::pow()``

See ``xo-unit/examples/ex6`` for code below

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 15

    #include "xo/unit/quantity.hpp"
    #include "xo/unit/quantity_iostream.hpp"
    #include <iostream>

    int
    main () {
        namespace u = xo::unit::units;
        namespace q = xo::unit::qty;
        using namespace std;

        /* 20% volatility over 250 days (approx number of trading days in one year) */
        auto q1 = q::volatility_250d(0.2);
        /* 10% volatility over 30 days */
        auto q2 = q::volatility_30d(0.1);

        auto sum = q1 + q2;
        auto prod = q1 * q2;

        static_assert(sum.abbrev() == flatstring("yr360^(-1/2)"));
        static_assert(prod.abbrev() == flatstring("yr360^-1"));

        std::cerr << "q1: " << q1 << std::endl;
        std::cerr << "q2: " << q2 << std::endl;
        std::cerr << "q1+q2: " << sum << std::endl;
        std::cerr << "q1*q2: " << prod << std::endl;
    }

with output:

.. code-block::

    q1: 0.2yr360^(-1/2)
    q2: 0.1mo^(-1/2)
    q1+q2: 0.54641yr360^(-1/2)
    q1*q2: 0.069282yr360^-1


Dynamic dimension
-----------------

If the dimension (or units) associated with a quantity are not known at compile-time,
use ``xo::qty::xquantity`` instead of ``xo::qty::quantity``.

See ``xo-unit/example/ex8`` for code below

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 10-12

    #include "xo/unit/xquantity.hpp"
    #include "xo/unit/xquantity_iostream.hpp"
    #include <iostream>

    int
    main () {
        using namespace xo::qty;
        namespace u = xo::qty::u;

        xquantity qty1(7, u::foot);
        xquantity qty2(6.0, u::inch);
        xquantity qty3 = qty1 + qty2;

        std::cerr << "qty1: " << qty1 << std::endl;
        std::cerr << "qty2: " << qty2 << std::endl;
        std::cerr << "qty3: " << qty3 << std::endl;

        /* rescale to mm */
        xquantity res = qty3.rescale(xo::qty::nu::millimeter);

        /* 2286mm */
        std::cerr << "res: " << res << std::endl;
    }

Here ``u::foot`` and ``u::inch`` are literals,
but they could have been read from console input or another runtime-only context.
