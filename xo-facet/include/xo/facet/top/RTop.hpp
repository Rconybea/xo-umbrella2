/** @file RTop.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/facet/top/ATop.hpp"
#include "xo/facet/top/ITop_Any.hpp"
#include "xo/facet/RRouter.hpp"

namespace xo::facet {
    /** @class RTop **/
    template <typename Object>
    struct RTop : public Object {
    private:
        using O = Object;
    public:
        using DataPtr = Object::DataPtr;
        using typeseq = xo::facet::typeseq;

        RTop() {}
        RTop(Object::DataPtr data) : Object{std::move(data)} {}

        /** constructor for an obj<ATop> instance with interface @p iface and data @p ddata.
         *  Actual implementation type unknown at compile time -> runtime polymorphism.
         **/
        RTop(const ATop * iface, void * ddata)
        requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
        : Object(iface, ddata) {}

        bool _has_null_vptr() const noexcept { return O::iface()->_has_null_vptr(); }
        typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
        void _drop() const noexcept { O::iface()->_drop(O::data()); }

        static bool _valid;
    };

    template <typename Object>
    bool
    RTop<Object>::_valid = facet::valid_object_router<Object>();

    template <typename Object>
    struct RoutingFor<ATop, Object> {
        using RoutingType = RTop<Object>;
    };
} /*namespace xo::facet*/

/* end RTop.hpp */
