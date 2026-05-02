/** @file IGCObjectVisitor_Xfer.hpp
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
 *
 *  variables:
 *    {facet_hpp_fname} -> GCObjectVisitor.hpp
 *    {impl_hpp_subdir} -> gc
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> gc
 *    {abstract_facet_fname} -> AGCObjectVisitor.hpp
 **/

#pragma once

#include "AGCObjectVisitor.hpp"
#include <xo/alloc2/Generation.hpp>
#include <xo/alloc2/role.hpp>
#include <xo/alloc2/VisitReason.hpp>
#include <xo/arena/AllocInfo.hpp>

namespace xo {
namespace mm {
    /** @class IGCObjectVisitor_Xfer
     **/
    template <typename DRepr, typename IGCObjectVisitor_DRepr>
    class IGCObjectVisitor_Xfer : public AGCObjectVisitor {
    public:
        /** @defgroup mm-gcobjectvisitor-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IGCObjectVisitor_DRepr;
        /** integer identifying a type **/
        using typeseq = AGCObjectVisitor::typeseq;
        ///@}

        /** @defgroup mm-gcobjectvisitor-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AGCObjectVisitor

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        AllocInfo alloc_info(Copaque data, void * addr)  const override {
            return I::alloc_info(_dcast(data), addr);
        }
        Generation generation_of(Copaque data, Role r, const void * addr)  const  noexcept override {
            return I::generation_of(_dcast(data), r, addr);
        }

        // non-const methods
        void * alloc_copy(Opaque data, std::byte * src)  const override {
            return I::alloc_copy(_dcast(data), src);
        }
        void visit_child(Opaque data, VisitReason reason, AGCObject * iface, void ** pp_data)  const  noexcept override {
            return I::visit_child(_dcast(data), reason, iface, pp_data);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup mm-gcobjectvisitor-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IGCObjectVisitor_DRepr>
    xo::facet::typeseq
    IGCObjectVisitor_Xfer<DRepr, IGCObjectVisitor_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IGCObjectVisitor_DRepr>
    bool
    IGCObjectVisitor_Xfer<DRepr, IGCObjectVisitor_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AGCObjectVisitor,
                                              IGCObjectVisitor_Xfer>();

} /*namespace mm */
} /*namespace xo*/

/* end IGCObjectVisitor_Xfer.hpp */
