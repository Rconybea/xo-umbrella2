/** @file IGCObject_DList.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DList.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DList.json5]
**/

#include "list/IGCObject_DList.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DList::shallow_size(const DList & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DList::shallow_move(DList & self, obj<ACollector> gc) noexcept -> Opaque
        {
            return self.shallow_move(gc);
        }
        auto
        IGCObject_DList::forward_children(DList & self, obj<ACollector> gc) noexcept -> void
        {
            self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DList.cpp */
