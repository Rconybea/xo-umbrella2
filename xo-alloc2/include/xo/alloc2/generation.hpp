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
        static constexpr uint32_t c_max_generation = 16;

        /** @class generation
         *  @brief type-safe generation number
         **/
        struct generation {
            using value_type = std::uint32_t;

            constexpr generation() = default;
            explicit constexpr generation(value_type x) : value_{x} {}

            static generation nursery() { return generation{0}; }
            static generation sentinel() { return generation(c_max_generation); }

            bool is_sentinel() const noexcept { return value_ == c_max_generation; }

            constexpr operator value_type() const { return value_; }

            generation & operator++() { ++value_; return *this; }

            std::uint32_t value_ = 0;
        };

        inline bool operator==(generation lhs, generation rhs) {
            return lhs.value_ == rhs.value_;
        }

        inline bool operator<(generation lhs, generation rhs) {
            return lhs.value_ < rhs.value_;
        }

        inline bool operator>(generation lhs, generation rhs) {
            return lhs.value_ > rhs.value_;
        }

    }
}

/* end generation.hpp */
