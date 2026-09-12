/** @file IReflectable_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Reflectable.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Reflectable.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> Reflectable.hpp
 *    {impl_hpp_subdir} -> detail
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> detail
 *    {abstract_facet_fname} -> AReflectable.hpp
 **/

#pragma once

#include "AReflectable.hpp"

namespace xo {
namespace reflect {
    /** @class IReflectable_Xfer
     **/
    template <typename DRepr, typename IReflectable_DRepr>
    class IReflectable_Xfer : public AReflectable {
    public:
        /** @defgroup reflect-reflectable-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IReflectable_DRepr;
        /** integer identifying a type **/
        using typeseq = AReflectable::typeseq;
        using obj_AReflectable = AReflectable::obj_AReflectable;
        ///@}

        /** @defgroup reflect-reflectable-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AReflectable

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup reflect-reflectable-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IReflectable_DRepr>
    xo::facet::typeseq
    IReflectable_Xfer<DRepr, IReflectable_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IReflectable_DRepr>
    bool
    IReflectable_Xfer<DRepr, IReflectable_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AReflectable,
                                              IReflectable_Xfer>();

} /*namespace reflect */
} /*namespace xo*/

/* end IReflectable_Xfer.hpp */
