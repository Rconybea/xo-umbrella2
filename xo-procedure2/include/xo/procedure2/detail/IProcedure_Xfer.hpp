/** @file IProcedure_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Procedure.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Procedure.json5]
 **/

#pragma once

#include "RuntimeContext.hpp"
#include <xo/gc/GCObject.hpp>

namespace xo {
namespace scm {
    /** @class IProcedure_Xfer
     **/
    template <typename DRepr, typename IProcedure_DRepr>
    class IProcedure_Xfer : public AProcedure {
    public:
        /** @defgroup scm-procedure-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IProcedure_DRepr;
        /** integer identifying a type **/
        using typeseq = AProcedure::typeseq;
        using AGCObject = AProcedure::AGCObject;
        ///@}

        /** @defgroup scm-procedure-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AProcedure

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        bool is_nary(Copaque data)  const  noexcept override {
            return I::is_nary(_dcast(data));
        }
        std::int32_t n_args(Copaque data)  const  noexcept override {
            return I::n_args(_dcast(data));
        }

        // non-const methods
        obj<AGCObject> apply_nocheck(Opaque data, obj<ARuntimeContext> rcx, const DArray * args)  override {
            return I::apply_nocheck(_dcast(data), rcx, args);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup scm-procedure-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IProcedure_DRepr>
    xo::facet::typeseq
    IProcedure_Xfer<DRepr, IProcedure_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IProcedure_DRepr>
    bool
    IProcedure_Xfer<DRepr, IProcedure_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AProcedure,
                                              IProcedure_Xfer>();

} /*namespace scm */
} /*namespace xo*/

/* end IProcedure_Xfer.hpp */
