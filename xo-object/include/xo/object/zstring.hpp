/** @file zstring.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "object.hpp"
//#include <cstdint>

namespace xo {
    namespace obj {
        struct zstring {
            /* need to count trailing \0 */
            std::size_t alloc_size() const { return sizeof(zstring) + len_ + 1; }

            /** number of characters in string,  not counting trailing null **/
            std::int32_t len_;
            /** must use placement new to create **/
            char data_[];
        };
    } /*namespace obj*/
} /*namespace xo*/


/** end zstring.hpp **/
