/** @file IPrintable_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Printable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Printable.json5]
 **/

#pragma once

#include <xo/indentlog/print/ppindentinfo.hpp>

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
        ///@}

        /** @defgroup print-printable-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from APrintable

        // const methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        bool pretty(Copaque data, const ppindentinfo & ppii)  override {
            return I::pretty(_dcast(data), ppii);
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