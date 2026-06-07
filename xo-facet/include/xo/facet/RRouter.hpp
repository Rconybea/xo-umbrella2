/** @file RRouter.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "OObject.hpp"

namespace xo {
    namespace facet {
        template <typename RRouter>
        consteval bool valid_object_router()
        {
            static_assert(sizeof(RRouter) >= sizeof(RRouter::ObjectType),
                              "Router type must inherit Router::ObjectType");
            static_assert(std::is_convertible_v<RRouter, typename RRouter::ObjectType>,
                              "Router type must inherit Router::ObjectType");
            static_assert(requires { typename RRouter::ObjectType; },
                              "Router type must provide typename Router::ObjectType");
            static_assert(valid_object_traits<RRouter::ObjectType>,
                              "Router::ObjectType must satisfy objectmodel traits");
            static_assert(std::is_standard_layout_v<RRouter>,
                              "Router must have standard laayout, i.e. no virtual methods."
                              " Virtual methods belong in OObject::AbstractInterface*>");
            return true;
        };

        /**
         *  template <typename Object>
         *  struct RMyFacet : public Object {
         *      using ObjectType = Object;
         *
         *      RObject() = default;
         *      RObject(Object::DataPtr data) : Object{data} {}
         *
         *      void something() const { return Object::iface()->something(Object::data()); }
         *      int andalso(double somearg) const { return Object::iface()->andalso(Object::data(), somearg); }
         *  };
         *
         *  template <typename Object>
         *  struct RoutingFor<AMyFacet, Object> {
         *      using RoutingType = RMyFacet<Object>;
         *  };
         **/
        template <typename AFacet, typename Object>
        requires abstract_facet<AFacet>
        struct RoutingFor;

        template <typename AFacet, typename Object>
        using RoutingType = RoutingFor<AFacet, Object>::RoutingType;
    }
} /*namespace xo*/

/* end RRouter.hpp */
