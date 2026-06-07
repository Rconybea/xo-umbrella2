/** @file ex1.cpp **/

#include "xo/ratio/ratio_iostream.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/hex.hpp"
#include <iostream>

namespace {
    using xo::xtag;
    using xo::hex_view;

#ifdef NOT_USING
    template <std::size_t N>
    xo::flatstring<N>
    flatstring_from_int(int x)
    {
        XO_SCOPE(log, always);

        constexpr size_t buf_z = 20;

        bool negative_flag = (x < 0);
        std::size_t i = buf_z;
        char buf[buf_z];
        std::fill_n(buf, N, '\0');

        if (negative_flag)
            x = -x;

        buf[--i] = '\0';

        while ((i > 0) && (x != 0)) {
            buf[--i] = ('0' + x % 10);
            x = x / 10;
        }

        if ((i > 0) && negative_flag)
            buf[--i] = '-';

        char retv[N];
        std::fill_n(retv, N, '\0');
        std::copy_n(buf + i, buf_z - i, retv);

        log && log(xtag("i",i), xtag("buf[i..]", hex_view(buf+i, buf+buf_z, true)));

        return retv;
    }
#endif

    template <typename Ratio, std::size_t N>
    constexpr xo::flatstring<N>
    ratio_to_str(Ratio x) noexcept
    {
        if (x.is_integer()) {
            return xo::flatstring<N>::from_int(x.num());
        } else {
            constexpr auto num_str = xo::flatstring<N>::from_int(x.num());
            constexpr auto den_str = xo::flatstring<N>::from_int(x.den());

            constexpr auto tmp = flatstring_concat("(", num_str, "/", den_str, ")");

            return tmp;
        }
    }

    /* verify can use ratio instance as template argument */
    using ratio64_t = xo::ratio::ratio<int64_t>;

    template<ratio64_t Ratio>
    constexpr double my_ratio_approx = Ratio.num_ / static_cast<double>(Ratio.den_);

}

int
main() {
    using xo::ratio::make_ratio;
    using xo::ratio::ratio;
    using xo::ratio::ratio_concept;
    using xo::flatstring;
    using namespace std;

    static_assert(my_ratio_approx<ratio64_t(1, 2)> == 0.5);
    static_assert(my_ratio_approx<ratio64_t{}> == 0.0);

    constexpr auto r1 = make_ratio(2, 3);
    cerr << "r1=make_ratio(2,3): " << r1 << endl; // output <ratio 2/3>

    static_assert(r1.num() == 2);
    static_assert(r1.den() == 3);

    static_assert(r1.compare(r1, r1) == 0);
    static_assert(xo::ratio::detail::op_aux_type<decltype(r1), decltype(r1)>::compare(r1,r1) == 0);

    static_assert(r1 == r1);
    static_assert(!(r1 != r1));
    static_assert(r1 != 0);
    static_assert(r1 != 1);
    static_assert(r1 != 2);
    static_assert(r1 != 3);

    static_assert(r1 >= r1);
    static_assert(r1 <= r1);

    static_assert(r1 > 0);
    static_assert(r1 >= 0);
    static_assert(r1 < 1);
    static_assert(r1 <= 1);

    constexpr auto r2 = make_ratio(2, 4);
    cerr << "r2=make_ratio(2,4): " << r2 << endl; // output <ratio 1/2>

    static_assert(r2.num() == 1);
    static_assert(r2.den() == 2);

    static_assert(r2 == r2);
    static_assert(r2 != r1);
    static_assert(!(r2 > r1));
    static_assert(!(r2 >= r1));
    static_assert(r2 <= r1);
    static_assert(r2 < r1);
    static_assert(r1 > r2);
    static_assert(r1 >= r2);
    static_assert(!(r1 < r2));
    static_assert(!(r1 <= r2));

    constexpr auto r3 = make_ratio(2, 3) - make_ratio(1, 2);
    cerr << "r3=r1-r2: " << r1 - r2 << endl;  // output <ratio 1/6>

    static_assert(r3.num() == 1);
    static_assert(r3.den() == 6);

    static_assert(r3 == r3);
    static_assert(r3 != 0);
    static_assert(r3 != 1);
    static_assert(r3 < r2);
    static_assert(r3 <= r2);
    static_assert(r3 < r1);
    static_assert(r3 <= r1);

    constexpr auto r4 = r1 + r2;
    cerr << "r4=r1+r2: " << r1 + r2 << endl;  // output <ratio 7/6>

    static_assert(r4.num() == 7);
    static_assert(r4.den() == 6);
    static_assert(r4 > 1);
    static_assert(r4 < 2);

    constexpr auto r5 = r1 + 3;
    cerr << "r5=r1+3: " << r5 << endl; // output <ratio 11/3>

    static_assert(r5.num() == 11);
    static_assert(r5.den() == 3);

    constexpr auto r6 = 3 + r1;
    cerr << "r5=3+r1: " << r6 << endl; // output <ratio 11/3>

    static_assert(r6.num() == 11);
    static_assert(r6.den() == 3);

    static_assert(r5 == r6);
    static_assert(r5 > 3);
    static_assert(r5 < 4);
    static_assert(r5 > r1);

    constexpr auto r7 = r6 - 3;
    cerr << "r7=r6-3: " << r7 << endl; // output <ratio 2/3>

    static_assert(r7 == r1);
    static_assert(r7 >= r1);
    static_assert(r7 <= r1);

    constexpr auto r8 = 3 - r6;
    cerr << "r8=3-r6: " << r8 << endl; // output <ratio -2/3>

    static_assert(r8 == r8);
    static_assert(r8 > -1);
    static_assert(r8 < 0);
    static_assert(-1 < r8);
    static_assert(-1 <= r8);
    static_assert(0 >= r8);

    constexpr auto r9 = r8 * r8;
    cerr << "r9=r8*r8: " << r9 << endl; // output <ratio 4/9>

    static_assert(r9 == make_ratio(4, 9));

    constexpr auto r10 = r9 * 9;
    cerr << "r10=r9*9: " << r10 << endl; // output <ratio 4/1>

    static_assert(r10 == 4);
    static_assert(r10.convert_to<int>() == 4);

    constexpr auto r11 = r9 * 3;
    cerr << "r11=r9*3: " << r11 << endl; // output <ratio 4/3>

    static_assert(r11 == make_ratio(4, 3));
    static_assert(r11.convert_to<int>() == 1);

    constexpr auto r12 = 9 * r9;
    cerr << "r12=9*r9: " << r12 << endl;

    static_assert(r12 == r10);
    static_assert(r12 == make_ratio(4, 1));
    static_assert(r12.convert_to<int>() == 4);

    constexpr auto r13 = 3 * r9;
    cerr << "r13=3*r9: " << r13 << endl; // output <ratio 4/3>

    static_assert(r13 == make_ratio(4, 3));
    static_assert(r13 == make_ratio(-4, -3));
    static_assert(r13 == r11);
    static_assert(r13.convert_to<int>() == 1);

    constexpr auto r14 = r9 / r9;
    cerr << "r14=r9/r9: " << r14 << endl; // output <ratio 4/3>

    static_assert(r14 == 1);
    static_assert(r14.convert_to<int>() == 1);

    constexpr auto r15 = r9 / r8;
    cerr << "r15=r9/r8: " << r15 << endl; // (4/9) / (-2/3) = (4/9) * (3/-2) = 12/-18 = -2/3

    static_assert(r15 == make_ratio(-2, 3));
    static_assert(r15 == make_ratio(2, -3));

    constexpr auto r16 = r9 / 2;
    cerr << "r16=r9/2: " << r16 << endl;

    static_assert(r16 == ratio(2, 9));
    static_assert(!r16.is_integer());

    constexpr auto r17 = 2 / r9;
    cerr << "r17=2/r9: " << r17 << endl; // 9/2

    static_assert(r17 == ratio(9, 2));
    static_assert(!r17.is_integer());

    constexpr auto s17_num_str = flatstring<20>::from_int(r17.num());
    static_assert(s17_num_str == flatstring("9"));
    constexpr auto s17_den_str = flatstring<20>::from_int(r17.den());
    static_assert(s17_den_str == flatstring("2"));
    constexpr auto s17_str = flatstring_concat(flatstring_concat(flatstring("("),
                                                                 s17_num_str),
                                               flatstring_concat(flatstring("/"),
                                                                 s17_den_str),
                                               flatstring(")"));
    cerr << "s17_str=" << s17_str << endl;

    //constexpr auto s17 = ratio_to_str<decltype(r17), 20>(r17);

    constexpr auto r18 = r12 / r8;
    cerr << "r18=r12/r8: " << r12/r8 << endl;

    static_assert(r18.is_integer());

    constexpr auto r19 = r18.power(2);
    cerr << "r19=r18^2: " << r19 << endl;

    static_assert(r19.is_integer());
    static_assert(r19 == ratio(36, 1));

    constexpr auto r20 = r17.power(-3);
    cerr << "r20=r17^-3: " << r20 << endl;

    static_assert(!r20.is_integer());
    static_assert(r20 == ratio(8, 729));

    //cerr << flatstring_from_int<10>(1) << endl;

    constexpr auto s20 = flatstring<10>::from_int(-123);
    cerr << "s20=" << s20 << endl;

    static_assert(s20.size() > 0);
    static_assert(s20 == flatstring("-123"));


    /* verify constexpr working */
    static_assert(ratio<int>(2,3).num() == 2);
    static_assert(ratio<int>(2,3).den() == 3);
    static_assert(make_ratio(-1,2).num() == -1);
    static_assert(make_ratio(-1,2).den() == 2);
    static_assert(make_ratio(-2,4).num() == -1);
    static_assert(make_ratio(-2,4).den() == 2);
    static_assert(make_ratio(1,-2).num() == -1);
    static_assert(make_ratio(1,-2).den() == 2);
    static_assert(make_ratio(-4,-6).num() == 2);
    static_assert(make_ratio(-4,-6).den() == 3);
}
