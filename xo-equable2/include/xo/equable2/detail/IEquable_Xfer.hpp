/** @file IEquable_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Equable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Equable.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> Equable.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> AEquable.hpp
 **/

#pragma once

#include "AEquable.hpp"

namespace xo {
namespace equable {
    /** @class IEquable_Xfer
     **/
    template <typename DRepr, typename IEquable_DRepr>
    class IEquable_Xfer : public AEquable {
    public:
        /** @defgroup equable-equable-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IEquable_DRepr;
        /** integer identifying a type **/
        using typeseq = AEquable::typeseq;
        using obj_AEquable = AEquable::obj_AEquable;
        ///@}

        /** @defgroup equable-equable-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AEquable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup equable-equable-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IEquable_DRepr>
    xo::facet::typeseq
    IEquable_Xfer<DRepr, IEquable_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IEquable_DRepr>
    bool
    IEquable_Xfer<DRepr, IEquable_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AEquable,
                                              IEquable_Xfer>();

} /*namespace equable */
} /*namespace xo*/

/* end IEquable_Xfer.hpp */
