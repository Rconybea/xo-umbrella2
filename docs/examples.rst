.. _examples:

.. toctree
   :maxdepth: 2

Examples
========

Compile-time unit inference
---------------------------

.. code-block:: cpp

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int main() {
        namespace u = xo::unit::units;
        namespace qty = xo::unit::qty;
        using xo::unit::quantity;
        using namespace std;

        auto t = qty::milliseconds(10);
        auto m = qty::kilograms(2.5);
        auto a = m / (t * t);

        static_assert(same_as<decltype(t), quantity<u::millisecond, int>>);
        static_assert(same_as<decltype(m), quantity<u::kilogram, double>>);
        static_assert(sizeof(t) == sizeof(int));
        static_assert(sizeof(m) == sizeof(double));
        static_assert(sizeof(a) == sizeof(double));

        cerr << "t: " << t << ", m: " << m << ", a: " << a << endl;
    }

with output:

.. code-block::

    t: 10ms, m: 2.5kg, m.t^-2: 0.025kg.ms^-2

Remarks:

*  The ``xo-unit`` system runs entirely at compile time;  there's no runtime overhead.
*  No runtime overhead includes construction of literal strings such as ``kg.ms^-2``
   (this is once place implementation requires c++20)
*  Units are sticky: since we expressed ``t`` in milliseconds and ``m`` in kilograms, result is in the same terms.
*  Unit ordering is sticky.  Mass appears on the left in printed value of ``a`` because it was on the left-hand side of ``operator/``
*  Example omits verifying ``decltype(a)``,  to keep output small.
*  Conversion factors are exact (provided dimensions are limited to integer powers).
   Exact conversion involves no loss of precision.

Explicit scale conversion
-------------------------

Can convert between compatible units explictly:

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 11-12

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int
    main () {
        namespace u = xo::unit::units;
        namespace qty = xo::unit::qty;
        using namespace std;

        auto t1 = qty::milliseconds(25.0);
        auto t1_usec = t1.with_unit<u::microsecond>();
        auto t1_sec = t1.with_unit<u::second>();

        cerr << "t1: " << t1 << ", t1_usec: " << t1_usec << ", t1_sec: " << t1_sec << endl;
    }

with output:

.. code-block::

    t1: 25ms, t1_usec: 25000us, t1_sec: 0.025s


Scale conversion triggered by assignment
----------------------------------------

One way to convert units is by assignment:

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 10-11

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int main() {
        namespace u = xo::unit;
        namespace qty = xo::units::qty;
        using xo::unit::quantity;
        using namespace std;

        quantity<units::second> t = qty::milliseconds(10);
        quantity<units::gram> m = qty::kilograms(2.5);
        auto a = m / (t * t);

        cerr << "t: " << t << ", m: " << m << ", a: " << a << endl;
    }

with output:

.. code-block::

    t: 0.01s, m: 2500g, m.t^-2: 2.5e+07g.s^-2

Remarks:

*  Assignment to ``t`` converted to representation ``double``.
   We could have used :code:`quantity<unit::second, int>` to convert (possibly rounding down)
   representation to `int`.

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
