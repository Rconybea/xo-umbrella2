/** @file role.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <cstdint>

namespace xo {
    namespace mm {
        enum class role {
            /** GC will keep one to-space for each generation.
             *  Application allocs always happen in to-space.
             **/
            to_space,
            /** During normal operation from-space is empty.
             *  During collection phase itself,
             *  to-space and from-space are exchanged,
             *  with from-space becoming the space to be collected
             **/
            from_space,
            /** counts entries **/
            N
        };

        constexpr uint32_t role2int(role x) { return static_cast<uint32_t>(x); }

    } /*namespace mm*/
} /*namespace xo*/

/* end role,hpp */
