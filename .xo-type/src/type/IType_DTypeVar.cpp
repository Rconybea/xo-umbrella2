/** @file IType_DTypeVar.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DTypeVar.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DTypeVar.json5]
**/

#include "typevar/IType_DTypeVar.hpp"

namespace xo {
    namespace scm {
        auto
        IType_DTypeVar::metatype(const DTypeVar & self) noexcept -> Metatype
        {
            return self.metatype();
        }

        auto
        IType_DTypeVar::repr_td(const DTypeVar & self) noexcept -> TypeDescr
        {
            return self.repr_td();
        }

        auto
        IType_DTypeVar::is_equal_to(const DTypeVar & self, const obj_AType & y) -> bool
        {
            return self.is_equal_to(y);
        }

        auto
        IType_DTypeVar::is_subtype_of(const DTypeVar & self, const obj_AType & y) -> bool
        {
            return self.is_subtype_of(y);
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IType_DTypeVar.cpp */
