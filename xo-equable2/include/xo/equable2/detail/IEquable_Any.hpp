/** @file IEquable_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Equable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Equable.json5]
 **/

#pragma once

#include "AEquable.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace equable { class IEquable_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::equable::AEquable,
                           DVariantPlaceholder>
{
    using ImplType = xo::equable::IEquable_Any;
};

}
}

namespace xo {
namespace equable {

    /** @class IEquable_Any
     *  @brief AEquable implementation for empty variant instance
     **/
    class IEquable_Any : public AEquable {
    public:
        /** @defgroup equable-equable-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using obj_AEquable = AEquable::obj_AEquable;

        ///@}
        /** @defgroup equable-equable-any-methods **/
        ///@{

        const AEquable * iface() const { return std::launder(this); }

        // from AEquable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods

        // nonconst methods

        ///@}

    private:
        /** @defgraoup equable-equable-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup equable-equable-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace equable */
} /*namespace xo */

/* IEquable_Any.hpp */
