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
        namespace u = xo::unit;
        namespace qty = u::qty;
        using namespace std;

        auto t = qty::milliseconds(10);
        auto m = qty::kilograms(2.5);
        auto a = m / (t * t);

        static_assert(same_as<decltype(t), u::quantity<u::millisecond, int>>);
        static_assert(same_as<decltype(m), u::quantity<u::kilogram, double>>);
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

Scale conversion triggered by assignment
----------------------------------------

One way to convert units is by assignment:

.. code-block:: cpp
   :linenos:
   :emphasize-lines: 9-10

    #include "xo/unit/quantity.hpp"
    #include <iostream>

    int main() {
        namespace u = xo::unit;
        namespace qty = xo::units::qty;
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

In representing a particular quantity,
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
