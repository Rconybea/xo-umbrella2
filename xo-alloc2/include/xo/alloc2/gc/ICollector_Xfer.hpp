/** @file ICollector_Xfer.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Collector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Collector.json5]
 *
 *  variables:
 *    {facet_hpp_fname} -> Collector.hpp
 *    {impl_hpp_subdir} -> gc
 *    {facet_ns1} -> xo
 *    {facet_detail_subdir} -> gc
 *    {abstract_facet_fname} -> ACollector.hpp
 **/

#pragma once

#include "ACollector.hpp"
#include <xo/alloc2/Allocator_basic.hpp>
#include <xo/alloc2/Generation.hpp>
#include <xo/alloc2/role.hpp>

namespace xo {
namespace mm {
    /** @class ICollector_Xfer
     **/
    template <typename DRepr, typename ICollector_DRepr>
    class ICollector_Xfer : public ACollector {
    public:
        /** @defgroup mm-collector-xfer-type-traits **/
        ///@{
        /** actual implementation (not generated; often delegates to DRepr) **/
        using Impl = ICollector_DRepr;
        /** integer identifying a type **/
        using typeseq = ACollector::typeseq;
        using size_type = ACollector::size_type;
        ///@}

        /** @defgroup mm-collector-xfer-methods **/
        ///@{

        static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
        static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

        // from ACollector

        // builtin methods
        typeseq _typeseq() const noexcept override { return s_typeseq; }
        void _drop(Opaque d) const noexcept override { _dcast(d).~DRepr(); }

        // const methods
        size_type allocated(Copaque data, Generation g, Role r)  const  noexcept override {
            return I::allocated(_dcast(data), g, r);
        }
        size_type committed(Copaque data, Generation g, Role r)  const  noexcept override {
            return I::committed(_dcast(data), g, r);
        }
        size_type reserved(Copaque data, Generation g, Role r)  const  noexcept override {
            return I::reserved(_dcast(data), g, r);
        }
        std::int32_t locate_address(Copaque data, const void * addr)  const  noexcept override {
            return I::locate_address(_dcast(data), addr);
        }
        bool contains(Copaque data, Role r, const void * addr)  const  noexcept override {
            return I::contains(_dcast(data), r, addr);
        }
        bool is_type_installed(Copaque data, typeseq tseq)  const  noexcept override {
            return I::is_type_installed(_dcast(data), tseq);
        }
        bool report_statistics(Copaque data, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept override {
            return I::report_statistics(_dcast(data), report_mm, error_mm, output);
        }
        bool report_object_types(Copaque data, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept override {
            return I::report_object_types(_dcast(data), report_mm, error_mm, output);
        }
        bool report_object_ages(Copaque data, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept override {
            return I::report_object_ages(_dcast(data), report_mm, error_mm, output);
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
        void * alloc_copy(Opaque data, std::byte * src)  override {
            return I::alloc_copy(_dcast(data), src);
        }

        ///@}

    private:
        using I = Impl;

    public:
        /** @defgroup mm-collector-xfer-member-vars **/
        ///@{

        /** typeseq for template parameter DRepr **/
        static typeseq s_typeseq;
        /** true iff satisfies facet implementation **/
        static bool _valid;

        ///@}
    };

    template <typename DRepr, typename ICollector_DRepr>
    xo::facet::typeseq
    ICollector_Xfer<DRepr, ICollector_DRepr>::s_typeseq
      = xo::facet::typeseq::id<DRepr>();

    template <typename DRepr, typename ICollector_DRepr>
    bool
    ICollector_Xfer<DRepr, ICollector_DRepr>::_valid
      = xo::facet::valid_facet_implementation<ACollector,
                                              ICollector_Xfer>();

} /*namespace mm */
} /*namespace xo*/

/* end ICollector_Xfer.hpp */
