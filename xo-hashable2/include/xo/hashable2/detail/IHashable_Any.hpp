/** @file IHashable_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Hashable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Hashable.json5]
 **/

#pragma once

#include "AHashable.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace hashable { class IHashable_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::hashable::AHashable,
                           DVariantPlaceholder>
{
    using ImplType = xo::hashable::IHashable_Any;
};

}
}

namespace xo {
namespace hashable {

    /** @class IHashable_Any
     *  @brief AHashable implementation for empty variant instance
     **/
    class IHashable_Any : public AHashable {
    public:
        /** @defgroup hashable-hashable-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using obj_AHashable = AHashable::obj_AHashable;

        ///@}
        /** @defgroup hashable-hashable-any-methods **/
        ///@{

        const AHashable * iface() const { return std::launder(this); }

        // from AHashable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods

        // nonconst methods

        ///@}

    private:
        /** @defgraoup hashable-hashable-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup hashable-hashable-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace hashable */
} /*namespace xo */

/* IHashable_Any.hpp */
