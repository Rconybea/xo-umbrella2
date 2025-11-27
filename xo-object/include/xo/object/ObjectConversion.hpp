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
    }
}

/* end ObjectConversion.hpp */
