/** @file symbol.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "zstring.hpp"
//#include <cstdint>

namespace xo {
    namespace obj {
        class symbol {
        public:
            symbol(const symbol & x) : name_{x.name_} {}

            zstring * name_;
        };
    } /*namespace obj*/
} /*namespace xo*/


/** end symbol.hpp **/
