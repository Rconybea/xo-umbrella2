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
    /** convenience template for gc object copy **/
    template <typename T>
    void * alloc_copy_for(const T * src) noexcept {
        return O::iface()->alloc_copy(O::data(), (std::byte *)const_cast<T *>(src));
    }

    /** convenience template for move-constructible T (this is common) **/
    template <typename T>
    T * std_move_for(T * src) noexcept {
        void * mem = this->alloc_copy_for(src);
        if (mem) {
            return new (mem) T(std::move(*src));
        }
        return nullptr;
    }

    /** forward faceted object pointer in place. Defined in GCObject.hpp to avoid #include cycle **/
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
    bool _has_null_vptr() const noexcept { return O::iface()->_has_null_vptr(); }
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    size_type allocated(Generation g, Role r)  const  noexcept {
        return O::iface()->allocated(O::data(), g, r);
    }
    size_type committed(Generation g, Role r)  const  noexcept {
        return O::iface()->committed(O::data(), g, r);
    }
    size_type reserved(Generation g, Role r)  const  noexcept {
        return O::iface()->reserved(O::data(), g, r);
    }
    std::int32_t locate_address(const void * addr)  const  noexcept {
        return O::iface()->locate_address(O::data(), addr);
    }
    bool contains(Role r, const void * addr)  const  noexcept {
        return O::iface()->contains(O::data(), r, addr);
    }
    bool is_type_installed(typeseq tseq)  const  noexcept {
        return O::iface()->is_type_installed(O::data(), tseq);
    }
    bool report_statistics(obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept {
        return O::iface()->report_statistics(O::data(), report_mm, error_mm, output);
    }
    bool report_object_types(obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept {
        return O::iface()->report_object_types(O::data(), report_mm, error_mm, output);
    }
    bool report_object_ages(obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept {
        return O::iface()->report_object_ages(O::data(), report_mm, error_mm, output);
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
    void * alloc_copy(std::byte * src)  {
        return O::iface()->alloc_copy(O::data(), src);
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
