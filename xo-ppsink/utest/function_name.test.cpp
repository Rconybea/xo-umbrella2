/** @file function_name.test.cpp **/

#include <xo/ppsink/function_name.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>

namespace ut {
    using xo::pp::put_function_name;
    using xo::pp::FlatSink;
    using xo::FunctionStyle;
    using std::stringstream;

    /* render @p pretty at @p style into a string via a FlatSink */
    static std::string
    styled(FunctionStyle style, std::string_view pretty) {
        stringstream ss;
        FlatSink sink(ss);
        put_function_name(sink, style, pretty);
        return ss.str();
    }

    TEST_CASE("function_name.streamlined", "[function_name]") {
        /* free function: no class qualifier -> just the name */
        REQUIRE(styled(FunctionStyle::streamlined, "int fib(int)") == "fib");
        /* class method: keep Class::method, drop return type + args + const */
        REQUIRE(styled(FunctionStyle::streamlined,
                       "double Quadratic::evaluate(double) const") == "Quadratic::evaluate");
        /* namespaced: drop enclosing namespaces above the class */
        REQUIRE(styled(FunctionStyle::streamlined,
                       "void xo::ns::Foo::bar()") == "Foo::bar");
        /* templated class: drop template args + the [with ...] footnote */
        REQUIRE(styled(FunctionStyle::streamlined,
                       "int Foo<T>::baz(int) [with T = int]") == "Foo::baz");
        /* operator(): keep the trailing () of the operator name */
        REQUIRE(styled(FunctionStyle::streamlined,
                       "double Quadratic::operator()(double) const") == "Quadratic::operator()");
    }

    TEST_CASE("function_name.simple", "[function_name]") {
        REQUIRE(styled(FunctionStyle::simple, "int fib(int)") == "fib");
        REQUIRE(styled(FunctionStyle::simple,
                       "double Quadratic::evaluate(double) const") == "evaluate");
        REQUIRE(styled(FunctionStyle::simple, "void xo::ns::Foo::bar()") == "bar");
    }

    TEST_CASE("function_name.literal-and-pretty", "[function_name]") {
        REQUIRE(styled(FunctionStyle::literal, "int fib(int)") == "int fib(int)");
        REQUIRE(styled(FunctionStyle::pretty, "int fib(int)") == "[int fib(int)]");
    }
} /*namespace ut*/

/* end function_name.test.cpp */
