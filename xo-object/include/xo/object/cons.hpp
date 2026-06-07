/** @file cons.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#Include "variant.hpp"
//#include <cstdint>

namespace xo {
    namespace var {
        struct cons {
            variant car_;
            variant cdr_;
        };
    } /*namespace var*/
} /*namespace xo*/


/** end cons.hpp **/
