/** @file AllocHeader.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace xo {
    namespace mm {
        struct AllocHeader {
            using repr_type = std::uintptr_t;
            using size_type = std::size_t;

            explicit AllocHeader(repr_type x) : repr_{x} {}

            repr_type repr_;
        };

        static_assert(sizeof(AllocHeader) == sizeof(AllocHeader::repr_type));
        static_assert(std::is_standard_layout_v<AllocHeader>);
    }
}

/* end AllocHeader.hpp */
