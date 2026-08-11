/** @file IHashable_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Hashable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Hashable.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> Hashable.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> AHashable.hpp
 **/

#pragma once

#include "AHashable.hpp"

namespace xo {
namespace hashable {
    /** @class IHashable_Xfer
     **/
    template <typename DRepr, typename IHashable_DRepr>
    class IHashable_Xfer : public AHashable {
    public:
        /** @defgroup hashable-hashable-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IHashable_DRepr;
        /** integer identifying a type **/
        using typeseq = AHashable::typeseq;
        using obj_AHashable = AHashable::obj_AHashable;
        ///@}

        /** @defgroup hashable-hashable-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AHashable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup hashable-hashable-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IHashable_DRepr>
    xo::facet::typeseq
    IHashable_Xfer<DRepr, IHashable_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IHashable_DRepr>
    bool
    IHashable_Xfer<DRepr, IHashable_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AHashable,
                                              IHashable_Xfer>();

} /*namespace hashable */
} /*namespace xo*/

/* end IHashable_Xfer.hpp */
