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

            explicit constexpr generation(value_type x) : value_{x} {}

            static generation nursery() { return generation{0}; }

            constexpr operator value_type() const { return value_; }

            generation & operator++() { ++value_; return *this; }

            std::uint32_t value_;
        };
    }
}

/* end generation.hpp */
