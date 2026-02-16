/** @file IResourceVisitor_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ResourceVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ResourceVisitor.json5]
 **/

#pragma once

#include "Allocator.hpp"

namespace xo {
namespace mm {
    /** @class IResourceVisitor_Xfer
     **/
    template <typename DRepr, typename IResourceVisitor_DRepr>
    class IResourceVisitor_Xfer : public AResourceVisitor {
    public:
        /** @defgroup mm-resourcevisitor-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IResourceVisitor_DRepr;
        /** integer identifying a type **/
        using typeseq = AResourceVisitor::typeseq;
        using size_type = AResourceVisitor::size_type;
        ///@}

        /** @defgroup mm-resourcevisitor-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AResourceVisitor

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        void on_allocator(Copaque data, obj<AAllocator> mm)  const  noexcept override {
            return I::on_allocator(_dcast(data), mm);
        }

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup mm-resourcevisitor-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IResourceVisitor_DRepr>
    xo::facet::typeseq
    IResourceVisitor_Xfer<DRepr, IResourceVisitor_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IResourceVisitor_DRepr>
    bool
    IResourceVisitor_Xfer<DRepr, IResourceVisitor_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AResourceVisitor,
                                              IResourceVisitor_Xfer>();

} /*namespace mm */
} /*namespace xo*/

/* end IResourceVisitor_Xfer.hpp */
