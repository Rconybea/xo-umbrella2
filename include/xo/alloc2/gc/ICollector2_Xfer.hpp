/** @file ICollector2_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Collector2.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Collector2.json5]
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/generation.hpp>
#include <xo/alloc2/role.hpp>

namespace xo {
namespace mm {
    /** @class ICollector2_Xfer
     **/
    template <typename DRepr, typename ICollector2_DRepr>
    class ICollector2_Xfer : public ACollector2 {
    public:
        /** @defgroup mm-collector2-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = ICollector2_DRepr;
        /** integer identifying a type **/
        using typeseq = ACollector2::typeseq;
        using size_type = ACollector2::size_type;
        ///@}

        /** @defgroup mm-collector2-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from ACollector2

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        size_type allocated(Copaque data, Generation g, role r)  const  noexcept override {
            return I::allocated(_dcast(data), g, r);
        }
        size_type committed(Copaque data, Generation g, role r)  const  noexcept override {
            return I::committed(_dcast(data), g, r);
        }
        size_type reserved(Copaque data, Generation g, role r)  const  noexcept override {
            return I::reserved(_dcast(data), g, r);
        }
        bool contains(Copaque data, role r, const void * addr)  const  noexcept override {
            return I::contains(_dcast(data), r, addr);
        }
        bool is_type_installed(Copaque data, typeseq tseq)  const  noexcept override {
            return I::is_type_installed(_dcast(data), tseq);
        }

        // non-const methods
        bool install_type(Opaque data, const AGCObject & iface)  override {
            return I::install_type(_dcast(data), iface);
        }
        void add_gc_root_poly(Opaque data, obj<AGCObject> * p_root)  override {
            return I::add_gc_root_poly(_dcast(data), p_root);
        }
        void remove_gc_root_poly(Opaque data, obj<AGCObject> * p_root)  override {
            return I::remove_gc_root_poly(_dcast(data), p_root);
        }
        void request_gc(Opaque data, Generation upto)  override {
            return I::request_gc(_dcast(data), upto);
        }
        void assign_member(Opaque data, void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs)  override {
            return I::assign_member(_dcast(data), parent, p_lhs, rhs);
        }
        void forward_inplace(Opaque data, AGCObject * lhs_iface, void ** lhs_data)  override {
            return I::forward_inplace(_dcast(data), lhs_iface, lhs_data);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup mm-collector2-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename ICollector2_DRepr>
    xo::facet::typeseq
    ICollector2_Xfer<DRepr, ICollector2_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename ICollector2_DRepr>
    bool
    ICollector2_Xfer<DRepr, ICollector2_DRepr>::_valid
      = xo::facet::valid_facet_implementation<ACollector2,
                                              ICollector2_Xfer>();

} /*namespace mm */
} /*namespace xo*/

/* end ICollector2_Xfer.hpp */
