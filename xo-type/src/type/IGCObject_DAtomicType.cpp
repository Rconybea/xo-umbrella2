/** @file IGCObject_DAtomicType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DAtomicType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DAtomicType.json5]
**/

#include "atomic/IGCObject_DAtomicType.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DAtomicType::shallow_size(const DAtomicType & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DAtomicType::shallow_copy(const DAtomicType & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DAtomicType::forward_children(DAtomicType & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DAtomicType.cpp */
