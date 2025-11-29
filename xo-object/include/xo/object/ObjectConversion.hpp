/** @file ObjectConversion.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Object.hpp"

namespace xo {
    namespace obj {
        template <typename T>
        struct ObjectConversion {
            static gp<Object> to_object(gc::IAlloc * mm, const T & x) = delete;
            static T from_object(gc::IAlloc * mm, gp<Object> x) = delete;
        };

        /** see specializations:
         *    ObjectConversion<bool>
         * in object/Boolean.hpp
         *
         *    ObjectConversion<int64_t>
         *    ObjectConversion<int32_t>
         *    ObjectConversion<int16_t>
         * in object/Integer.hpp
         **/
    }
}

/* end ObjectConversion.hpp */
