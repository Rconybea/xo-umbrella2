/** @file quantity2.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "bpu_array.hpp"

namespace xo {
    namespace qty {
        /** @class quantity
         *  @brief represent a scalar quantity with attached units.  enforce dimensional consistency.
         *
         *  Constexpr implementation, can compute units at compile time
         **/
        template <typename Repr = double, typename Int = std::int64_t>
        class quantity2 {
        public:
            using repr_type = Repr;

        private:
        };
    } /*namespace qty*/
} /*namespace xo*/


/** end quantity2.hpp **/
