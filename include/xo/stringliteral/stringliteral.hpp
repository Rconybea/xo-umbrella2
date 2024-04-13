/** @file stringliteral.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include <string_view>
#include <algorithm>
#include <cstdint>

namespace xo {
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

    /** @brief all_same_v<T1, .., Tn> is true iff types T1 = .. = Tn
     **/
    template < typename First, typename... Rest >
    constexpr auto
    all_same_v = std::conjunction_v< std::is_same<First, Rest>... >;

    /** @brief concatenate string literals
     *
     *  NOTE: this isn't constexpr in clang16
     **/
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

            std::copy_n(arg.c_str(), count, result.value_ + pos);
            pos += count;
        };

        (detail_concat(args), ...);

        //return stringliteral("");
        return result;
    }

#ifdef NOT_USING
    template <std::size_t N1, std::size_t N2>
    constexpr auto
    stringliteral_compare(stringliteral<N1> && s1, stringliteral<N2> && s2)
    {
        return std::string_view(s1.value_) <=> std::string_view(s2.value_);
    }
#endif

    template <std::size_t N1, std::size_t N2>
    constexpr auto
    stringliteral_compare(const stringliteral<N1> & s1, const stringliteral<N2> & s2)
    {
        return std::string_view(s1.value_) <=> std::string_view(s2.value_);
    }
} /*namespace xo*/


/** end stringliteral.hpp **/
