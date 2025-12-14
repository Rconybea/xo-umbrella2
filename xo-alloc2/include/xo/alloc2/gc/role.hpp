/** @file role.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <cstdint>

namespace xo {
    namespace mm {
        static constexpr uint32_t c_n_role = 2;

        struct role {
            using value_type = std::uint32_t;

            explicit constexpr role(value_type x) : role_{x} {}

            static constexpr role to_space() { return role{0}; }
            static constexpr role from_space() { return role{1}; }

            operator value_type() const { return role_; }

            std::uint32_t role_;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end role,hpp */
