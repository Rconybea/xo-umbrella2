/** @file IType_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Type.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Type.json5]
 **/

#pragma once

#include "AType.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace scm { class IType_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::scm::AType,
                           DVariantPlaceholder>
{
    using ImplType = xo::scm::IType_Any;
};

}
}

namespace xo {
namespace scm {

    /** @class IType_Any
     *  @brief AType implementation for empty variant instance
     **/
    class IType_Any : public AType {
    public:
        /** @defgroup scm-type-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using obj_AType = AType::obj_AType;
        using TypeDescr = AType::TypeDescr;

        ///@}
        /** @defgroup scm-type-any-methods **/
        ///@{

        const AType * iface() const { return std::launder(this); }

        // from AType

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods
        [[noreturn]] Metatype metatype(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] TypeDescr repr_td(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] bool is_equal_to(Copaque, const obj_AType &)  const override { _fatal(); }
        [[noreturn]] bool is_subtype_of(Copaque, const obj_AType &)  const override { _fatal(); }

        // nonconst methods

        ///@}

    private:
        /** @defgraoup scm-type-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup scm-type-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace scm */
} /*namespace xo */

/* IType_Any.hpp */
