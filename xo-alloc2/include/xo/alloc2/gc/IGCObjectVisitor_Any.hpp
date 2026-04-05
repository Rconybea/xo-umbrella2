/** @file IGCObjectVisitor_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObjectVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObjectVisitor.json5]
 **/

#pragma once

#include "AGCObjectVisitor.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace mm { class IGCObjectVisitor_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::mm::AGCObjectVisitor,
                           DVariantPlaceholder>
{
    using ImplType = xo::mm::IGCObjectVisitor_Any;
};

}
}

namespace xo {
namespace mm {

    /** @class IGCObjectVisitor_Any
     *  @brief AGCObjectVisitor implementation for empty variant instance
     **/
    class IGCObjectVisitor_Any : public AGCObjectVisitor {
    public:
        /** @defgroup mm-gcobjectvisitor-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;

        ///@}
        /** @defgroup mm-gcobjectvisitor-any-methods **/
        ///@{

        const AGCObjectVisitor * iface() const { return std::launder(this); }

        // from AGCObjectVisitor

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] void _drop(Opaque) const noexcept override { _fatal(); }

        // const methods

        // nonconst methods
        [[noreturn]] void visit_child(Opaque, AGCObject *, void **)  const  noexcept override;

        ///@}

    private:
        /** @defgraoup mm-gcobjectvisitor-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup mm-gcobjectvisitor-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace mm */
} /*namespace xo */

/* IGCObjectVisitor_Any.hpp */
