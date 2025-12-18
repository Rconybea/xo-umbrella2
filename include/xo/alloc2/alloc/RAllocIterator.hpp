/** @file RAllocIterator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AAllocIterator.hpp"
#include <xo/facet/RRouter.hpp>
//#include <string>

namespace xo {
    namespace mm {
        /* @class RAllocIterator */
        template <typename Object>
        struct RAllocIterator : public Object {
        private:
            using O = Object;
        public:
            using ObjectType = Object;
            using DataPtr = Object::DataPtr;

            RAllocIterator() {}
            RAllocIterator(Object::DataPtr data) : Object{std::move(data)} {}

            int32_t _typeseq() const noexcept { return O::iface()->_typeseq(); }

            static bool _valid;
        };

        template <typename Object>
        bool
        RAllocIterator<Object>::_valid = facet::valid_object_router<Object>();
    } /*namespace mm*/

    namespace facet {
        template <typename Object>
        struct RoutingFor<xo::mm::AAllocIterator, Object> {
            using RoutingType = xo::mm::RAllocIterator<Object>;
        };
    }
} /*namespace xo*/

/* end RAllocIterator.hpp */
