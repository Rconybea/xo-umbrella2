/** @file ISequence_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Sequence.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Sequence.json5]
 **/

#pragma once

#include <xo/gc/GCObject.hpp>

namespace xo {
namespace scm {
    /** @class ISequence_Xfer
     **/
    template <typename DRepr, typename ISequence_DRepr>
    class ISequence_Xfer : public ASequence {
    public:
        /** @defgroup scm-sequence-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = ISequence_DRepr;
        /** integer identifying a type **/
        using typeseq = ASequence::typeseq;
        using size_type = ASequence::size_type;
        using AGCObject = ASequence::AGCObject;
        ///@}

        /** @defgroup scm-sequence-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from ASequence

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        bool is_empty(Copaque data)  const  noexcept override {
            return I::is_empty(_dcast(data));
        }
        bool is_finite(Copaque data)  const  noexcept override {
            return I::is_finite(_dcast(data));
        }
        obj<AGCObject> at(Copaque data, size_type index)  const override {
            return I::at(_dcast(data), index);
        }

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup scm-sequence-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename ISequence_DRepr>
    xo::facet::typeseq
    ISequence_Xfer<DRepr, ISequence_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename ISequence_DRepr>
    bool
    ISequence_Xfer<DRepr, ISequence_DRepr>::_valid
      = xo::facet::valid_facet_implementation<ASequence,
                                              ISequence_Xfer>();

} /*namespace scm */
} /*namespace xo*/

/* end ISequence_Xfer.hpp */