/* @file ex1.cpp */

#include "xo/flatstring/flatstring.hpp"
//#include "xo/stringliteral/stringliteral_iostream.hpp"
//#include "xo/flatstring/experiment.hpp"
//#include "xo/stringliteral/string_view_concat.hpp"
#include <iostream>

int
main() {
    using namespace std;
    using xo::flatstring;
#ifdef WAITAMO
    using xo::stringliteral_compare;
#endif

#ifdef NOT_USING
    static_assert(foo1().x_ == 1);
    static_assert(foo1().y_ == 2);

    constexpr foo1 s1;

    static_assert(s1.x_ == 1);
    static_assert(s1.y_ == 2);

    constexpr foo2 s2;

    static_assert(s2.v_[0] == 'a');
    static_assert(s2.v_[1] == 'b');

    constexpr foo3<2> s3;

    static_assert(s3.v_[0] == 'a');
    static_assert(s3.v_[1] == 'b');

    constexpr foo4<6> s4("hello");

    constexpr foo5 s5("hello");

    static_assert(s5.v_[0] == 'h');
    static_assert(s5.v_[5] == '\0');

    constexpr foo6 s6("hello", ", world!");

    static_assert(s6.v_[0] == 'h');
    static_assert(s6.size() == 13);

    cerr << "s6=" << s6.c_str() << endl;

    /* z gives allocation size.  string size is z-1 */
    constexpr std::size_t z = concat_size("hello", ", world!", " What's up?");

    static_assert(z == 25);

    constexpr foo7<10> s7("Hello", ", world!", " What's up?");

    constexpr stringlit<10> s8("0123", "45678");

    static_assert(s8.size() == 9);
    constexpr std::size_t z8 = stringlit_capacity(s8);


    static_assert(sizeof("0123") == 5);
    static_assert(sizeof("45") == 3);
    static_assert(sizeof("78") == 3);

    static_assert(literal_strlen("0123") == 4);


    static_assert(z8 == 10);
#endif

#ifdef NOT_USING
    static_assert(count_size("0123") == 5);

    static_assert(count_size("0123", "45") == 7);
    static_assert(count_size("0123", "45", "67", "8") == 10);

    constexpr auto z9 = count_size("0123", "45", "78");

    static_assert(z9 == 9);

    constexpr auto z10 = foofn("0123");

    static_assert(z10 == 5);
#endif

#ifdef NOT_USING
    //constexpr auto z11 = foofn2("0123");

    //static_assert(z9 > 22);

    constexpr auto s9 = stringlit_make("0123", "456", "78");
    //constexpr auto s9 = stringlit_makepalooza("0123", "45678");

    static_assert(s9.size() == 9);

    constexpr auto s10 = stringlit_make("0", "123", "456", "78");

    static_assert(s10.size() == 9);

    cerr << s10.c_str() << endl;
#endif

#ifdef NOT_SUCCESSFUL
    constexpr auto s11 = stringlit_make("0", "1", "23", "456", "78");
#endif

#ifdef NOT_USING
    constexpr std::size_t z9 = stringlit_capacity(s9, s10);

    static_assert(z9 == 19);

    constexpr auto s12 = stringlit_cat(s9, s10);

    static_assert(s12.size() == 18);

    cerr << s12.c_str() << endl;

    constexpr auto s13 = stringlit_cat(s9, s10, s12);

    static_assert(s13.size() == 36);

    cerr << s13.c_str() << endl;
#endif

#ifdef NOT_USING
    static_assert(stringliteral_compare(s1, s1) == 0);

    cerr << s1 << endl;
#endif

    constexpr flatstring s14 = flatstring_concat(flatstring("foo"), flatstring("bar"));

    static_assert(s14.fixed_capacity == 7);
    static_assert(sizeof(s14) == 7);

    constexpr flatstring s15 = flatstring_concat(flatstring("hello"),
                                                 flatstring(", "),
                                                 flatstring("world"));
    static_assert(s15.fixed_capacity == 13);
    static_assert(sizeof(s15) == 13);

    constexpr auto s16 = xo::flatstring_concat(flatstring("foo"), flatstring("bar"));

    static_assert(s16.fixed_capacity == 7);

    constexpr auto cmp = flatstring_compare(s14, s14);

    static_assert(cmp == 0);

#ifdef WAITAMO
    constexpr stringliteral s2 = stringliteral_stringlit_make(stringliteral("hello"),
                                                      stringliteral(", world"));
#endif

#ifdef NOT_USING
    static constexpr string_view hello("hello");
    static constexpr string_view world(" world");

    static constexpr auto s3 = stringlit_make_v<hello, world>;

    static constexpr string_view hello_world("hello world");

    static_assert(s3 == hello_world);

    cerr << hello_world << endl;
#endif
}

/* end ex1.cpp */
