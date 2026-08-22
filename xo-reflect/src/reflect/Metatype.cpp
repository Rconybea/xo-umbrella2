/** @file Metatype.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "Metatype.hpp"

namespace xo::reflect {

    const char *
    metatype2str(Metatype x) {
        switch (x) {
        case Metatype::mt_invalid: return "invalid!";
        case Metatype::mt_atomic: return "atomic";
        case Metatype::mt_pointer: return "pointer";
        case Metatype::mt_vector: return "vector";
        case Metatype::mt_struct: return "struct";
        case Metatype::mt_function: return "function";
        }

        return "?metatype";
    }


} /*namespace xo::reflect*/

/* end Metatype.cpp */
