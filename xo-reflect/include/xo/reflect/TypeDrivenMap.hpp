/* @file TypeDrivenMap.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "TypeDescr.hpp"
#include <vector>

namespace xo {
    namespace reflect {
        /* represents a map :: TypeId -> Value */
        template<typename Value>
        class TypeDrivenMap {
        public:
            Value const * lookup(TypeId id) const { return this->lookup_slot(id); }

            Value * require(TypeId id) { return this->require_slot(id); }
            Value * require(TypeDescr td) { return this->require_slot(td->id()); }

        private:
            Value const * lookup_slot(TypeId id) const {
                if (this->contents_v_.size() <= id.id())
                    return nullptr;

                return &(this->contents_v_[id.id()]);
            } /*lookup_slot*/

            Value * require_slot(TypeId id) {
                if (this->contents_v_.size() <= id.id())
                    this->contents_v_.resize(id.id() + 1);

                return &(this->contents_v_[id.id()]);
            } /*require_slot*/

        private:
            /* since TypeId/s are unique, compact sequence numbers,
             * can efficiently store mapping to Values using a vector indexed by TypeId
             */
            std::vector<Value> contents_v_;
        }; /*TypeDrivenMap*/
    } /*namespace reflect*/
} /*namespace xo*/


/* end TypeDrivenMap.hpp */
