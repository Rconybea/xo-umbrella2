.. _pretty_impl:

Pretty Printer
==============

Abstraction tower for *xo-indentlog2* pretty-printer

.. ditaa::
    :--scale: 0.85

    +----------------------------------------------+
    |                 xo-indentlog2                |
	+----------------------------------------------+
	|                   xo-arena                   |
	+----------------+--------------+--------------+
    |                | xo-testutil  |              |
	| xo-reflectutil +--------------+ xo-randomgen |
    |                | xo-subsys    |              | 
    +----------------+--------------+--------------+
    |                  xo-timeutil                 |
    +----------------------------------------------+


.. ditaa::
    :--scale: 0.85

    +---------------+------------------------------+--------------+
	|      	   	    |         PrettyPrinter        |   LogState   |
	|               +------------------------------+--------------+
	|               |       PpState    	   	       | LogStreambuf |
	|   FlatSink    |       PpTokenStreambuf       |              |
	|               +----------------------+-------+--------------+
	|               |       PpToken        |       LogBuffer      |
	|               +----------------------+----------------------+
	|               |     PpTokenType      |                      |
	+---------------+-----+----------------+      LineState       |
	|        PpSink       |    PpConfig    |	                  |
    +---------------------+----------------+----------------------+


Polymorphism
------------

Our design uses runtime polymoprhism for output streams,
and compile-time polymorphism for objects being printed.

Sinks rely on inheritance:

.. code-block:: cpp

    namespace xo::pp {
	    class PpSink {
        public:
            virtual void put(std::string_view) = 0;
		    virtual void begin() = 0;
			virtual void split() = 0;
			virtual void end() = 0;

			// atomic fallback: format x with operator<< as one
			// opaque token.
			template <class T> void put_streamed(const T & x);
		};

		class PrettyPrinter : public PpSink { ... };
		class FlatSink : public PpSink { ... };
    }

We also rely on a custom streambuf

.. code-block:: cpp

    namespace xo::pp {
	    class PpTokenStreambuf : public std::streambuf { ... };
		class LogStreambuf : public std::streambuf { ... };
    }

`PpTokenStreambuf` support types that provides a `std::ostream` inserter:
when `PrettyPrinter` encounters a value of such a type it uses `PpTokenStreambuf`
to treat it as an implicit string token (`PpStringToken`), in such as way as to
avoiding an extra copy from a temporary stream.


Custom Printers
---------------

The pretty printer requires opt-in for each participating type.
*xo-indentlog2* provides a template

.. code-block:: cpp

    namespace xo::pp {
        template <class T> struct Printer;

		inline constexpr ... pretty {};
    }

To provide pretty-printing for a type, specialize **xo::pp::Printer**
in the **xo::pp** namespace:

.. code-block:: cpp

    #include <xo/indenlog2/print/Printer.hpp>

    namespace xo::pp {
        template <> struct Printer<Quadratic> {
		    static void print(PpSink & s, const Quadratic & x) {
			    s.put("Quadratic{");
				s.begin();
				s.split();
				pretty(s, x.a_);
				s.put(",");
				s.split();
				pretty(s, x.b_);
				s.put(",");
				s.split();
				pretty(s, x.c_);
				s.end();
				s.put(")");
		    }
	    };
    }

**pretty** is a CPO (customization point object) that falls back to *operator<<*
for atomic types that provide it.
