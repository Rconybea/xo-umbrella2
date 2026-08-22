/* @file Metatype.hpp */

#pragma once

#include <xo/ppsink/Prettifier.hpp>

namespace xo {
    namespace reflect {
        enum class Metatype { mt_invalid, mt_atomic, mt_pointer, mt_vector, mt_struct, mt_function };

        const char * metatype2str(Metatype x);
    } /*namespace reflect*/
} /*namespace xo*/

namespace xo::pp {
    template <>
    class Prettifier<xo::reflect::Metatype> {
    public:
        static void print(PpSink & sink, const xo::reflect::Metatype & x) {
            using xo::reflect::metatype2str;

            sink.put(metatype2str(x));
        }
    };
}

/* end Metatype.hpp */
