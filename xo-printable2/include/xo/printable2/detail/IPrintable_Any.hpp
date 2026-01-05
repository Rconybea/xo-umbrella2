/** @file IPrintable_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Printable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Printable.json5]
 **/

#pragma once

#include "APrintable.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace print { class IPrintable_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::print::APrintable,
                           DVariantPlaceholder>
{
    using ImplType = xo::print::IPrintable_Any;
};

}
}

namespace xo {
namespace print {

    /** @class IPrintable_Any
     *  @brief APrintable implementation for empty variant instance
     **/
    class IPrintable_Any : public APrintable {
    public:
        /** @defgroup print-printable-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;

        ///@}
        /** @defgroup print-printable-any-methods **/
        ///@{

        const APrintable * iface() const { return std::launder(this); }

        // from APrintable

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] bool pretty(Copaque, const ppindentinfo &)  override { _fatal(); }

        // nonconst methods

        ///@}

    private:
        /** @defgraoup print-printable-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgraoup print-printable-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace print */
} /*namespace xo */

/* IPrintable_Any.hpp */