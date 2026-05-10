/** @file role.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <array>
#include <cstdint>

namespace xo {
    namespace mm {
        static constexpr uint32_t c_n_role = 2;

        /** @brief identify GC half-spaces
         **/
        class Role {
        public:
            using value_type = std::uint32_t;

            explicit constexpr Role(value_type x) : role_{x} {}

            static constexpr Role to_space() { return Role{0}; }
            static constexpr Role from_space() { return Role{1}; }

            static constexpr std::array<Role, c_n_role> all() { return {{to_space(), from_space()}}; }

            static constexpr Role begin() { return Role{0}; }
            static constexpr Role end() { return Role{2}; }

            operator value_type() const { return role_; }

            Role next() const { return Role(role_ + 1); }

            value_type role_ = 0;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end role.hpp */
