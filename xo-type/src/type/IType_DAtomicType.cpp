/** @file IType_DAtomicType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DAtomicType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DAtomicType.json5]
**/

#include "atomic/IType_DAtomicType.hpp"

namespace xo {
    namespace scm {
        auto
        IType_DAtomicType::metatype(const DAtomicType & self) noexcept -> Metatype
        {
            return self.metatype();
        }

        auto
        IType_DAtomicType::repr_td(const DAtomicType & self) noexcept -> TypeDescr
        {
            return self.repr_td();
        }

        auto
        IType_DAtomicType::is_equal_to(const DAtomicType & self, const obj_AType & y) -> bool
        {
            return self.is_equal_to(y);
        }

        auto
        IType_DAtomicType::is_subtype_of(const DAtomicType & self, const obj_AType & y) -> bool
        {
            return self.is_subtype_of(y);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IType_DAtomicType.cpp */
