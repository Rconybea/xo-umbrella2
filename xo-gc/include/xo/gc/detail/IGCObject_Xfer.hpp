/** @file IGCObject_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObject.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObject.json5]
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/gc/Collector.hpp>
#include <cstdint>
#include <cstddef>

namespace xo {
namespace mm {
    /** @class IGCObject_Xfer
     **/
    template <typename DRepr, typename IGCObject_DRepr>
    class IGCObject_Xfer : public AGCObject {
    public:
        /** @defgroup mm-gcobject-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IGCObject_DRepr;
        /** integer identifying a type **/
        using typeseq = AGCObject::typeseq;
        using size_type = AGCObject::size_type;
        using AAllocator = AGCObject::AAllocator;
        using ACollector = AGCObject::ACollector;
        ///@}

        /** @defgroup mm-gcobject-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AGCObject

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        size_type shallow_size(Copaque data)  const  noexcept override {
            return I::shallow_size(_dcast(data));
        }
        Opaque shallow_copy(Copaque data, obj<AAllocator> mm)  const  noexcept override {
            return I::shallow_copy(_dcast(data), mm);
        }

        // non-const methods
        size_type forward_children(Opaque data, obj<ACollector> gc)  const  noexcept override {
            return I::forward_children(_dcast(data), gc);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup mm-gcobject-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IGCObject_DRepr>
    xo::facet::typeseq
    IGCObject_Xfer<DRepr, IGCObject_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IGCObject_DRepr>
    bool
    IGCObject_Xfer<DRepr, IGCObject_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AGCObject,
                                              IGCObject_Xfer>();

} /*namespace mm */
} /*namespace xo*/

/* end IGCObject_Xfer.hpp */
