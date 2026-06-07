/** @file RCollector2.hpp
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

#include "ACollector2.hpp"

namespace xo {
namespace mm {

/** @class RCollector2
 **/
template <typename Object>
class RCollector2 : public Object {
private:
    using O = Object;

public:
    /** @defgroup mm-collector2-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using size_type = ACollector2::size_type;
    ///@}

    /** @defgroup mm-collector2-router-ctors **/
    ///@{
    RCollector2() {}
    RCollector2(Object::DataPtr data) : Object{std::move(data)} {}
    RCollector2(const ACollector2 * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup mm-collector2-router-methods **/
    ///@{

    // explicit injected content

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
    /** @defgroup mm-collector2-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RCollector2<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace mm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::mm::ACollector2, Object> {
        using RoutingType = xo::mm::RCollector2<Object>;
    };
} }

/* end RCollector2.hpp */
