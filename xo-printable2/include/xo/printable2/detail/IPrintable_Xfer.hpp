/** @file IPrintable_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Printable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Printable.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> Printable.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> APrintable.hpp
 **/

#pragma once

#include "APrintable.hpp"
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <xo/ppsink/PpSink.hpp>

namespace xo {
namespace print {
    /** @class IPrintable_Xfer
     **/
    template <typename DRepr, typename IPrintable_DRepr>
    class IPrintable_Xfer : public APrintable {
    public:
        /** @defgroup print-printable-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IPrintable_DRepr;
        /** integer identifying a type **/
        using typeseq = APrintable::typeseq;
        using ppindentinfo = APrintable::ppindentinfo;
        using PpSink = APrintable::PpSink;
        ///@}

        /** @defgroup print-printable-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from APrintable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        bool pretty_deprecated(Copaque data, const ppindentinfo & ppii)  const override {
            return I::pretty_deprecated(_dcast(data), ppii);
        }
        void pretty(Copaque data, PpSink & sink)  const override {
            return I::pretty(_dcast(data), sink);
        }

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup print-printable-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IPrintable_DRepr>
    xo::facet::typeseq
    IPrintable_Xfer<DRepr, IPrintable_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IPrintable_DRepr>
    bool
    IPrintable_Xfer<DRepr, IPrintable_DRepr>::_valid
      = xo::facet::valid_facet_implementation<APrintable,
                                              IPrintable_Xfer>();

} /*namespace print */
} /*namespace xo*/

/* end IPrintable_Xfer.hpp */
