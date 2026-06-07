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
        /** @brief per-alloc header
         *
         *  Appears immediately before each allocation when
         *  ArenaConfig.store_header_flag_ is set.
         *
         *  See AllocInfo.hpp for encoding of @ref repr_
         **/
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
