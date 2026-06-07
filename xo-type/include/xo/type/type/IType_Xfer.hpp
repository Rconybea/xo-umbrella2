/** @file IType_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Type.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Type.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> Type.hpp
 *    {impl_hpp_subdir} -> type
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> type
 *    {abstract_facet_fname} -> AType.hpp
 **/

#pragma once

#include "AType.hpp"
#include <xo/type/Metatype.hpp>
#include <xo/reflect/TypeDescr.hpp>

namespace xo {
namespace scm {
    /** @class IType_Xfer
     **/
    template <typename DRepr, typename IType_DRepr>
    class IType_Xfer : public AType {
    public:
        /** @defgroup scm-type-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = IType_DRepr;
        /** integer identifying a type **/
        using typeseq = AType::typeseq;
        using obj_AType = AType::obj_AType;
        using TypeDescr = AType::TypeDescr;
        ///@}

        /** @defgroup scm-type-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from AType

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        Metatype metatype(Copaque data)  const  noexcept override {
            return I::metatype(_dcast(data));
        }
        TypeDescr repr_td(Copaque data)  const  noexcept override {
            return I::repr_td(_dcast(data));
        }
        bool is_equal_to(Copaque data, const obj_AType & y)  const override {
            return I::is_equal_to(_dcast(data), y);
        }
        bool is_subtype_of(Copaque data, const obj_AType & y)  const override {
            return I::is_subtype_of(_dcast(data), y);
        }

        // non-const methods

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup scm-type-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename IType_DRepr>
    xo::facet::typeseq
    IType_Xfer<DRepr, IType_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename IType_DRepr>
    bool
    IType_Xfer<DRepr, IType_DRepr>::_valid
      = xo::facet::valid_facet_implementation<AType,
                                              IType_Xfer>();

} /*namespace scm */
} /*namespace xo*/

/* end IType_Xfer.hpp */
