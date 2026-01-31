/** @file IGCObject_Any.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObject.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObject.json5]
 **/

#pragma once

#include "AGCObject.hpp"
#include <xo/facet/obj.hpp>

namespace xo { namespace mm { class IGCObject_Any; } }

namespace xo {
namespace facet {

template <>
struct FacetImplementation<xo::mm::AGCObject,
                           DVariantPlaceholder>
{
    using ImplType = xo::mm::IGCObject_Any;
};

}
}

namespace xo {
namespace mm {

    /** @class IGCObject_Any
     *  @brief AGCObject implementation for empty variant instance
     **/
    class IGCObject_Any : public AGCObject {
    public:
        /** @defgroup mm-gcobject-any-type-traits **/
        ///@{

        /** integer identifying a type **/
        using typeseq = xo::facet::typeseq;
        using size_type = AGCObject::size_type;
        using AAllocator = AGCObject::AAllocator;
        using ACollector = AGCObject::ACollector;

        ///@}
        /** @defgroup mm-gcobject-any-methods **/
        ///@{

        const AGCObject * iface() const { return std::launder(this); }

        // from AGCObject

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        [[noreturn]] size_type shallow_size(Copaque)  const  noexcept override { _fatal(); }
        [[noreturn]] Opaque shallow_copy(Copaque, obj<AAllocator>)  const  noexcept override { _fatal(); }

        // nonconst methods
        [[noreturn]] size_type forward_children(Opaque, obj<ACollector>)  const  noexcept override;

        ///@}

    private:
        /** @defgraoup mm-gcobject-any-private-methods **/
        ///@{

        [[noreturn]] static void _fatal();

        ///@}

    public:
        /** @defgroup mm-gcobject-any-member-vars **/
        ///@{

        static typeseq s_typeseq;
        static bool _valid;

        ///@}
    };

} /*namespace mm */
} /*namespace xo */

/* IGCObject_Any.hpp */