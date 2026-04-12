/** @file generation.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <cstddef>
#include <cstdint>

namespace xo {
    namespace mm {
        /** hard maximum number of generations **/
        static constexpr uint32_t c_max_generation = 3;

        /** @class generation
         *  @brief type-safe generation number
         **/
        class Generation {
        public:
            using value_type = std::uint32_t;

            constexpr Generation() = default;
            explicit constexpr Generation(value_type x) : value_{x} {}

            static Generation nursery() { return Generation{0}; }
            static Generation g0() { return Generation{0}; }
            static Generation g1() { return Generation{1}; }
            static Generation sentinel() { return Generation(c_max_generation); }

            bool is_sentinel() const noexcept { return value_ == c_max_generation; }

            constexpr operator value_type() const { return value_; }

            Generation & operator++() { ++value_; return *this; }

            std::uint32_t value_ = 0;
        };

        inline bool operator==(Generation lhs, Generation rhs) {
            return lhs.value_ == rhs.value_;
        }

        inline bool operator<(Generation lhs, Generation rhs) {
            return lhs.value_ < rhs.value_;
        }

        inline bool operator>(Generation lhs, Generation rhs) {
            return lhs.value_ > rhs.value_;
        }

    }
}

/* end generation.hpp */
