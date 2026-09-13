/** @file IReflectable_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Reflectable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Reflectable.json5]
 **/

#pragma once

#include "AReflectable.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace reflect { class IReflectable_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::reflect::AReflectable,
                           DVariantPlaceholder>
{
    using ImplType = xo::reflect::IReflectable_Any;
};

}
}

namespace xo {
namespace reflect {

    /** @class IReflectable_Any
     *  @brief AReflectable implementation for empty variant instance
     **/
    class IReflectable_Any : public AReflectable {
    public:
        /** @defgroup reflect-reflectable-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using obj_AReflectable = AReflectable::obj_AReflectable;
        using TaggedPtr = AReflectable::TaggedPtr;

        ///@}
        /** @defgroup reflect-reflectable-any-methods **/
        ///@{

        const AReflectable * iface() const { return std::launder(this); }

        // from AReflectable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods

        // nonconst methods
        [[noreturn]] TaggedPtr self_tp(Opaque)  override;

        ///@}

    private:
        /** @defgraoup reflect-reflectable-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup reflect-reflectable-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace reflect */
} /*namespace xo */

/* IReflectable_Any.hpp */
