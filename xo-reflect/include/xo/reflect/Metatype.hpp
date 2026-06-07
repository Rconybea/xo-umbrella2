/* @file Metatype.hpp */

#pragma once

#include <iostream>

namespace xo {
    namespace reflect {
        enum class Metatype { mt_invalid, mt_atomic, mt_pointer, mt_vector, mt_struct, mt_function };

        inline std::ostream & operator<<(std::ostream & os,
                                         Metatype x) {
            switch(x) {
            case Metatype::mt_invalid:
                os << "invalid!";
                break;
            case Metatype::mt_atomic:
                os << "atomic";
                break;
            case Metatype::mt_pointer:
                os << "pointer";
                break;
            case Metatype::mt_vector:
                os << "vector";
                break;
            case Metatype::mt_struct:
                os << "struct";
                break;
            case Metatype::mt_function:
                os << "function";
                break;
            default:
                os << "???";
            }
            return os;
        } /*operator<<*/

    } /*namespace reflect*/
} /*namespace xo*/

/* end Metatype.hpp */
