/* @file ex1.cpp */

#include "xo/stringliteral/stringliteral.hpp"
#include "xo/stringliteral/stringliteral_iostream.hpp"
#include "xo/stringliteral/string_view_concat.hpp"
#include <iostream>

int
main() {
    using namespace std;
    using xo::stringliteral;
    using xo::stringliteral_compare;

#ifdef NOT_USING
    constexpr stringliteral s1("hello");

    static_assert(stringliteral_compare(s1, s1) == 0);

    cerr << s1 << endl;

    constexpr stringliteral s2 = stringliteral_concat(stringliteral("hello"),
                                                      stringliteral(", world"));

#endif

    static constexpr string_view hello("hello");
    static constexpr string_view world(" world");

    static constexpr auto s2 = concat_v<hello, world>;

    static constexpr string_view hello_world("hello world");

    static_assert(s2 == hello_world);

    cerr << hello_world << endl;
}

/* end ex1.cpp */
