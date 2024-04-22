/* @file stringliteral.hpp */

#pragma once

#include <cmath>
#include <string_view>
#include <algorithm>
#include <cstdint>

namespace xo {
    namespace unit {

        // ----- stringliteral -----

        /** @class stringliteral
         *
         *  @brief class to represent a literal string at compile time,  for use as template argument
         **/
        template <std::size_t N>
        struct stringliteral {
            constexpr stringliteral() { if (N > 0) value_[0] = '\0'; }
            constexpr stringliteral(const char (&str)[N]) { std::copy_n(str, N, value_); }
            constexpr int size() const { return N; }

            constexpr char const * c_str() const { return value_; }

            char value_[N];
        };

        template < typename First, typename... Rest >
        constexpr auto
        all_same_v = std::conjunction_v< std::is_same<First, Rest>... >;

        /** args and result must be stringliteral **/
        template < typename... Ts>
        constexpr auto
        stringliteral_concat(Ts && ... args)
        {
#ifdef NOT_USING
            static_assert(all_same_v<std::decay_t<Ts>...>,
                          "string must share the same char type");

            using char_type = std::remove_const_t< std::remove_pointer_t < std::common_type_t < Ts... > > >;
#endif
            using char_type = char;

            /** n1: total number of bytes used by arguments **/
            constexpr size_t n1 = (sizeof(Ts) + ...);
            /** z1: each string arg has a null terminator included in its size,
             *      z1 is the number of arguments in parameter pack Ts,
             *      which equals the number of null terminators used
             **/
            constexpr size_t z1 = sizeof...(Ts);

            /** n:  number of chars in concatenated string.  +1 for final null **/
            constexpr size_t n
                = (n1 / sizeof(char_type)) - z1 + 1;

            stringliteral<n> result;

            size_t pos = 0;

            auto detail_concat = [ &pos, &result ](auto && arg) {
                constexpr auto count = (sizeof(arg) - sizeof(char_type)) / sizeof(char_type);

                std::copy_n(arg, count, result.value_ + pos);
                pos += count;
            };

            (detail_concat(args), ...);

            return result;
        }

        template <std::size_t N1, std::size_t N2>
        constexpr auto
        stringliteral_compare(stringliteral<N1> && s1, stringliteral<N2> && s2)
        {
            return std::string_view(s1.value_) <=> std::string_view(s2.value_);
        }

        // ----- literal_size -----

        /** @brief compute number of chars needed to stringify an int **/
        template < int d, bool signbit = std::signbit(d) >
        struct literal_size;

        template < int d >
        struct literal_size<d, true> {
            /* d < 0 */
            static constexpr int size = 1 + literal_size<-d, false>::size;
        };

        template < int d >
        struct literal_size<d, false> {
            /* d >= 0 */
            static constexpr int size = 1 + literal_size<d/10, false>::size;
        };

        template <> struct literal_size<0, false> { static constexpr int size = 1; };
        template <> struct literal_size<1, false> { static constexpr int size = 1; };
        template <> struct literal_size<2, false> { static constexpr int size = 1; };
        template <> struct literal_size<3, false> { static constexpr int size = 1; };
        template <> struct literal_size<4, false> { static constexpr int size = 1; };
        template <> struct literal_size<5, false> { static constexpr int size = 1; };
        template <> struct literal_size<6, false> { static constexpr int size = 1; };
        template <> struct literal_size<7, false> { static constexpr int size = 1; };
        template <> struct literal_size<8, false> { static constexpr int size = 1; };
        template <> struct literal_size<9, false> { static constexpr int size = 1; };

        template < int d >
        constexpr int literal_size_v = literal_size<d>::size;

        // ----- literal_from_digit -----

        constexpr auto /*stringliteral<2>*/ stringliteral_from_digit( int d ) {
            return stringliteral({ static_cast<char>('0' + d), '\0' });
        }

#ifdef NOT_YET_22mar24
        template < int d >
        struct literal_from_digit;

        template <> struct literal_from_digit<0> { static constexpr auto value = stringliteral("0"); };
        template <> struct literal_from_digit<1> { static constexpr auto value = stringliteral("1"); };
        template <> struct literal_from_digit<2> { static constexpr auto value = stringliteral("2"); };
        template <> struct literal_from_digit<3> { static constexpr auto value = stringliteral("3"); };
        template <> struct literal_from_digit<4> { static constexpr auto value = stringliteral("4"); };
        template <> struct literal_from_digit<5> { static constexpr auto value = stringliteral("5"); };
        template <> struct literal_from_digit<6> { static constexpr auto value = stringliteral("6"); };
        template <> struct literal_from_digit<7> { static constexpr auto value = stringliteral("7"); };
        template <> struct literal_from_digit<8> { static constexpr auto value = stringliteral("8"); };
        template <> struct literal_from_digit<9> { static constexpr auto value = stringliteral("9"); };

        template < int d >
        constexpr auto literal_from_digit_v() { return literal_from_digit<d>::value; }
#endif

        // ----- stringliteral_from_int -----

        template < int D, int N = literal_size_v<D>, bool signbit = std::signbit(D) >
        struct stringliteral_from_int;

        template < int D, int N = literal_size_v<D>, bool signbit = std::signbit(D) >
        constexpr auto stringliteral_from_int_v() { return stringliteral_from_int<D, N, signbit>::value; }

        template < int D >
        struct stringliteral_from_int< D, 1, false > {
            static constexpr auto value = stringliteral_from_digit(D);
        };

        template < int D, int N >
        struct stringliteral_from_int< D, N, false > {
            static constexpr auto _prefix = stringliteral_from_int_v< D / 10, N - 1, false >();
            static constexpr auto _suffix = stringliteral_from_digit(D % 10);

            static constexpr auto value = stringliteral_concat(_prefix.value_, _suffix.value_);
        };

        template < int D, int N >
        struct stringliteral_from_int< D, N, true > {
            static constexpr auto _suffix = stringliteral_from_int_v< -D, N - 1, false>();

            static constexpr auto value = stringliteral_concat("-", _suffix.value_);
        };

    } /*namespace unit*/
} /*namespace xo*/

/* end stringliteral.hpp */
