/** @file IRuntimeContext_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/RuntimeContext.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/RuntimeContext.json5]
 **/

#pragma once

#include <xo/stringtable2/StringTable.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/arena/MemorySizeInfo.hpp>

namespace xo {
namespace scm {
    /** @class IRuntimeContext_Xfer
     **/
    template <typename DRepr, typename IRuntimeContext_DRepr>
    class IRuntimeContext_Xfer : public ARuntimeContext {
    public:
        /** @defgroup scm-runtimecontext-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IRuntimeContext_DRepr;
        /** integer identifying a type **/
        using typeseq = ARuntimeContext::typeseq;
        using AAllocator = ARuntimeContext::AAllocator;
        using MemorySizeVisitor = ARuntimeContext::MemorySizeVisitor;
        ///@}

        /** @defgroup scm-runtimecontext-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from ARuntimeContext

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        obj<AAllocator> allocator(Copaque data)  const  noexcept override {
            return I::allocator(_dcast(data));
        }
        StringTable * stringtable(Copaque data)  const  noexcept override {
            return I::stringtable(_dcast(data));
        }
        void visit_pools(Copaque data, MemorySizeVisitor visitor)  const override {
            return I::visit_pools(_dcast(data), visitor);
        }

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup scm-runtimecontext-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IRuntimeContext_DRepr>
    xo::facet::typeseq
    IRuntimeContext_Xfer<DRepr, IRuntimeContext_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IRuntimeContext_DRepr>
    bool
    IRuntimeContext_Xfer<DRepr, IRuntimeContext_DRepr>::_valid
      = xo::facet::valid_facet_implementation<ARuntimeContext,
                                              IRuntimeContext_Xfer>();

} /*namespace scm */
} /*namespace xo*/

/* end IRuntimeContext_Xfer.hpp */
