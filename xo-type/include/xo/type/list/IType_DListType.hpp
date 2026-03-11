/** @file IType_DListType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DListType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DListType.json5]
 **/

#pragma once

#include "Type.hpp"
#include "DListType.hpp"

namespace xo { namespace scm { class IType_DListType; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::scm::AType,
                                   xo::scm::DListType>
        {
            using ImplType = xo::scm::IType_Xfer
              <xo::scm::DListType,
               xo::scm::IType_DListType>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IType_DListType
         **/
        class IType_DListType {
        public:
            /** @defgroup scm-type-dlisttype-type-traits **/
            ///@{
            using obj_AType = xo::scm::AType::obj_AType;
            using TypeDescr = xo::scm::AType::TypeDescr;
            using Copaque = xo::scm::AType::Copaque;
            using Opaque = xo::scm::AType::Opaque;
            ///@}
            /** @defgroup scm-type-dlisttype-methods **/
            ///@{
            // const methods
            /** category for this type **/
            static Metatype metatype(const DListType & self) noexcept;
            /** reflected representation for instances of this type **/
            static TypeDescr repr_td(const DListType & self) noexcept;
            /** true iff this type is equal to y **/
            static bool is_equal_to(const DListType & self, const obj_AType & y);
            /** true iff this is a subtype of y **/
            static bool is_subtype_of(const DListType & self, const obj_AType & y);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */