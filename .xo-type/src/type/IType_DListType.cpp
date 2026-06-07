/** @file IType_DListType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DListType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DListType.json5]
**/

#include "list/IType_DListType.hpp"

namespace xo {
    namespace scm {
        auto
        IType_DListType::metatype(const DListType & self) noexcept -> Metatype
        {
            return self.metatype();
        }

        auto
        IType_DListType::repr_td(const DListType & self) noexcept -> TypeDescr
        {
            return self.repr_td();
        }

        auto
        IType_DListType::is_equal_to(const DListType & self, const obj_AType & y) -> bool
        {
            return self.is_equal_to(y);
        }

        auto
        IType_DListType::is_subtype_of(const DListType & self, const obj_AType & y) -> bool
        {
            return self.is_subtype_of(y);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IType_DListType.cpp */
