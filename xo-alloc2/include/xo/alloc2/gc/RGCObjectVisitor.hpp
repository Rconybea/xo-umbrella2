/** @file RGCObjectVisitor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObjectVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObjectVisitor.json5]
 **/

#pragma once

#include "AGCObjectVisitor.hpp"

namespace xo {
namespace mm {

/** @class RGCObjectVisitor
 **/
template <typename Object>
class RGCObjectVisitor : public Object {
private:
    using O = Object;

public:
    /** @defgroup mm-gcobjectvisitor-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    ///@}

    /** @defgroup mm-gcobjectvisitor-router-ctors **/
    ///@{
    RGCObjectVisitor() {}
    RGCObjectVisitor(Object::DataPtr data) : Object{std::move(data)} {}
    RGCObjectVisitor(const AGCObjectVisitor * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup mm-gcobjectvisitor-router-methods **/
    ///@{

    // explicit injected content
    /** convenience: allocate copy for typed pointer **/
    template <typename T>
    void * alloc_copy_for(const T * src) noexcept {
        return O::iface()->alloc_copy(O::data(), (std::byte *)const_cast<T *>(src));
    }
    
    /** convenience: move typed pointer **/
    template <typename T>
    T * std_move_for(T * src) noexcept {
        void * mem = this->alloc_copy_for(src);
        if (mem) {
            return new (mem) T(std::move(*src));
        }
        return nullptr;
    }
    
    /** visit a gcobject child pointer in place.
        Defined in RCollector_aux.hpp to avoid #include cycle
        (for historical reasons - coul d be in RGCObject_aux.hpp?)
     **/
    template <typename DRepr>
    void visit_child(VisitReason reason, xo::facet::obj<AGCObject,DRepr> * p_obj);
    
    /** visit typed child data pointer in place.
        Defined in RGCObject.hpp to avoid #include cycle
     **/
    template <typename DRepr>
    void visit_child(VisitReason reason, DRepr ** pp_data);
    
    /** visit faceted object pointer stored using some facet
        other than AGCObject
     **/
    template <typename AFacet, typename DRepr>
    requires (!std::is_same_v<AFacet, AGCObject>)
    void visit_poly_child(VisitReason reason, obj<AFacet,DRepr> * p_pivot);
    

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    AllocInfo alloc_info(void * addr)  const {
        return O::iface()->alloc_info(O::data(), addr);
    }
    Generation generation_of(Role r, const void * addr)  const  noexcept {
        return O::iface()->generation_of(O::data(), r, addr);
    }

    // non-const methods (still const in router!)
    void * alloc_copy(std::byte * src)  {
        return O::iface()->alloc_copy(O::data(), src);
    }
    void visit_child(VisitReason reason, AGCObject * iface, void ** pp_data)  noexcept {
        return O::iface()->visit_child(O::data(), reason, iface, pp_data);
    }

    ///@}
    /** @defgroup mm-gcobjectvisitor-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RGCObjectVisitor<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace mm*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::mm::AGCObjectVisitor, Object> {
        using RoutingType = xo::mm::RGCObjectVisitor<Object>;
    };
} }

/* end RGCObjectVisitor.hpp */
