/** @file IAbstractEventProcessor_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/AbstractEventProcessor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/AbstractEventProcessor.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> AbstractEventProcessor.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> AAbstractEventProcessor.hpp
 **/

#pragma once

#include "AAbstractEventProcessor.hpp"
#include <xo/stringtable2/DString.hpp>

namespace xo {
namespace reactor {
    /** @class IAbstractEventProcessor_Xfer
     **/
    template <typename DRepr, typename IAbstractEventProcessor_DRepr>
    class IAbstractEventProcessor_Xfer : public AAbstractEventProcessor {
    public:
        /** @defgroup reactor-abstracteventprocessor-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IAbstractEventProcessor_DRepr;
        /** integer identifying a type **/
        using typeseq = AAbstractEventProcessor::typeseq;
        using obj_AAbstractEventProcessor = AAbstractEventProcessor::obj_AAbstractEventProcessor;
        using DString = AAbstractEventProcessor::DString;
        ///@}

        /** @defgroup reactor-abstracteventprocessor-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AAbstractEventProcessor

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        const DString * name(Copaque data)  const  noexcept override {
            return I::name(_dcast(data));
        }

        // non-const methods
        void set_name(Opaque data, const DString * name)  noexcept override {
            return I::set_name(_dcast(data), name);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup reactor-abstracteventprocessor-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IAbstractEventProcessor_DRepr>
    xo::facet::typeseq
    IAbstractEventProcessor_Xfer<DRepr, IAbstractEventProcessor_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IAbstractEventProcessor_DRepr>
    bool
    IAbstractEventProcessor_Xfer<DRepr, IAbstractEventProcessor_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AAbstractEventProcessor,
                                              IAbstractEventProcessor_Xfer>();

} /*namespace reactor */
} /*namespace xo*/

/* end IAbstractEventProcessor_Xfer.hpp */
