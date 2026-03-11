/** @file IType_DFunctionType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IType_DFunctionType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IType_DFunctionType.json5]
 **/

#pragma once

#include "Type.hpp"
#include "DFunctionType.hpp"

namespace xo { namespace scm { class IType_DFunctionType; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::scm::AType,
                                   xo::scm::DFunctionType>
        {
            using ImplType = xo::scm::IType_Xfer
              <xo::scm::DFunctionType,
               xo::scm::IType_DFunctionType>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IType_DFunctionType
         **/
        class IType_DFunctionType {
        public:
            /** @defgroup scm-type-dfunctiontype-type-traits **/
            ///@{
            using obj_AType = xo::scm::AType::obj_AType;
            using TypeDescr = xo::scm::AType::TypeDescr;
            using Copaque = xo::scm::AType::Copaque;
            using Opaque = xo::scm::AType::Opaque;
            ///@}
            /** @defgroup scm-type-dfunctiontype-methods **/
            ///@{
            // const methods
            /** category for this type **/
            static Metatype metatype(const DFunctionType & self) noexcept;
            /** reflected representation for instances of this type **/
            static TypeDescr repr_td(const DFunctionType & self) noexcept;
            /** true iff this type is equal to y **/
            static bool is_equal_to(const DFunctionType & self, const obj_AType & y);
            /** true iff this is a subtype of y **/
            static bool is_subtype_of(const DFunctionType & self, const obj_AType & y);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */