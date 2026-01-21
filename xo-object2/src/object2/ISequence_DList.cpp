/** @file ISequence_DList.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ISequence_DList.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ISequence_DList.json5]
**/

#include "list/ISequence_DList.hpp"

namespace xo {
    namespace scm {
        auto
        ISequence_DList::is_empty(const DList & self) noexcept -> bool
        {
            return self.is_empty();
        }

        auto
        ISequence_DList::is_finite(const DList & self) noexcept -> bool
        {
            return self.is_finite();
        }

        auto
        ISequence_DList::at(const DList & self, size_type index) -> obj<AGCObject>
        {
            return self.at(index);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end ISequence_DList.cpp */