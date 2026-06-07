/** @file ratio_reflect.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/reflectutil/reflect_struct_info.hpp"
#include "ratio.hpp"
//#include <cstdint>

namespace xo {
    namespace reflect {
        template <typename Int>
        REFLECT_BASE_STRUCT_INFO_TBODY(xo::ratio::ratio<Int>, 2);

        template <typename Int>
        REFLECT_STRUCT_MEMBER_INFO_TBODY(xo::ratio::ratio<Int>, 0, num);

        template <typename Int>
        REFLECT_STRUCT_MEMBER_INFO_TBODY(xo::ratio::ratio<Int>, 1, den);
    }
}

/** end ratio_reflect.hpp **/
