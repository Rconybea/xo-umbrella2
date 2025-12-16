/** @file object_age.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <cstdint>

namespace xo {
    namespace mm {
        /** hard maximum remembered object age **/
        static constexpr uint32_t c_max_object_age = 127;

        /** @class object_age
         *  @brief type-safe object age
         *
         *  Object age measured in number of garbage collections survived.
         **/
        struct object_age {
            using value_type = std::uint32_t;

            explicit object_age(value_type x) : value_{x} {}

            operator value_type() const { return value_; }

            std::uint32_t value_;
        };
    }
}

/* end object_age.hpp */
