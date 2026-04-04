/** @file IGCObject_DListType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DListType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DListType.json5]
**/

#include "list/IGCObject_DListType.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DListType::shallow_size(const DListType & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DListType::shallow_move(DListType & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_move(mm);
        }
        auto
        IGCObject_DListType::forward_children(DListType & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DListType.cpp */
