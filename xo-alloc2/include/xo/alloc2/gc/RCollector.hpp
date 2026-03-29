/** @file RCollector.hpp
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
 **/

#pragma once

#include "ACollector.hpp"

namespace xo {
namespace mm {

/** @class RCollector
 **/
template <typename Object>
class RCollector : public Object {
private:
    using O = Object;

public:
    /** @defgroup mm-collector-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using size_type = ACollector::size_type;
    ///@}

    /** @defgroup mm-collector-router-ctors **/
    ///@{
    RCollector() {}
    RCollector(Object::DataPtr data) : Object{std::move(data)} {}
    RCollector(const ACollector * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup mm-collector-router-methods **/
    ///@{

    // explicit injected content
    /** forward op in place. Defined in GCObject.hpp to avoid #include cycle **/
    template <typename DRepr>
    void forward_inplace(obj<AGCObject,DRepr> * p_obj);
    
    /** another convenience template for forwarding.
     *  Defined in RGCObject.hpp to avoid #include cycle.
    **/
    template <typename DRepr>
    void forward_inplace(DRepr ** pp_repr);
    
    /** convenience template where pointer requires pivot **/
    template <typename AFacet, typename DRepr>
    requires (!std::is_same_v<AFacet, AGCObject>)
    void forward_pivot_inplace(obj<AFacet,DRepr> * p_obj);
    
    /** add root @p p_root **/
    template<typename DRepr>
    void add_gc_root(obj<AGCObject, DRepr> * p_root) {
        O::iface()->add_gc_root_poly(O::data(), (obj<AGCObject> *)p_root);
    }
    
    /** remove root @p p_root **/
    template <typename DRepr>
    void remove_gc_root(obj<AGCObject, DRepr> * p_root) {
        O::iface()->remove_gc_root_poly(O::data(), (obj<AGCObject> *)p_root);
    }

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    size_type allocated(Generation g, role r)  const  noexcept {
        return O::iface()->allocated(O::data(), g, r);
    }
    size_type committed(Generation g, role r)  const  noexcept {
        return O::iface()->committed(O::data(), g, r);
    }
    size_type reserved(Generation g, role r)  const  noexcept {
        return O::iface()->reserved(O::data(), g, r);
    }
    bool contains(role r, const void * addr)  const  noexcept {
        return O::iface()->contains(O::data(), r, addr);
    }
    bool is_type_installed(typeseq tseq)  const  noexcept {
        return O::iface()->is_type_installed(O::data(), tseq);
    }
    bool report_statistics(obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept {
        return O::iface()->report_statistics(O::data(), report_mm, error_mm, output);
    }

    // non-const methods (still const in router!)
    bool install_type(const AGCObject & iface)  {
        return O::iface()->install_type(O::data(), iface);
    }
    void add_gc_root_poly(obj<AGCObject> * p_root)  {
        return O::iface()->add_gc_root_poly(O::data(), p_root);
    }
    void remove_gc_root_poly(obj<AGCObject> * p_root)  {
        return O::iface()->remove_gc_root_poly(O::data(), p_root);
    }
    void request_gc(Generation upto)  {
        return O::iface()->request_gc(O::data(), upto);
    }
    void assign_member(void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs)  {
        return O::iface()->assign_member(O::data(), parent, p_lhs, rhs);
    }
    void forward_inplace(AGCObject * lhs_iface, void ** lhs_data)  {
        return O::iface()->forward_inplace(O::data(), lhs_iface, lhs_data);
    }

    ///@}
    /** @defgroup mm-collector-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RCollector<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace mm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::mm::ACollector, Object> {
        using RoutingType = xo::mm::RCollector<Object>;
    };
} }

/* end RCollector.hpp */
