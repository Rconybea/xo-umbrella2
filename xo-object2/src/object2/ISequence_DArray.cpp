/** @file ISequence_DArray.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ISequence_DArray.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ISequence_DArray.json5]
**/

#include "array/ISequence_DArray.hpp"

namespace xo {
    namespace scm {
        auto
        ISequence_DArray::is_empty(const DArray & self) noexcept -> bool
        {
            return self.is_empty();
        }

        auto
        ISequence_DArray::is_finite(const DArray & self) noexcept -> bool
        {
            return self.is_finite();
        }

        auto
        ISequence_DArray::at(const DArray & self, size_type index) -> obj<AGCObject>
        {
            return self.at(index);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end ISequence_DArray.cpp */