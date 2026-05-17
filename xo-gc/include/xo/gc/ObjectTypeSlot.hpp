/** @file ObjectTypeSlot.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "GCObject.hpp"

namespace xo {
    namespace mm {

        /** @brief Object Interface
         *
         *  GC-object interface for a particular type.
         *  X1 maintains a table of these (X1Collector::object_types_)
         *  indexed by typeseq.
         *
         *  Using a wrapper here for searchability
         **/
        struct ObjectTypeSlot {
            ObjectTypeSlot() {}
            //explicit ObjectTypeSlot(AGCObject * iface) { this->store_iface(iface); }

            /** true iff this slot is empty **/
            bool is_null() const noexcept {
                return this->_iface()->_has_null_vptr();
            }

            bool is_occupied() const noexcept {
                return !this->is_null();
            }

            AGCObject * _iface() const noexcept {
                return std::launder((AGCObject *)&iface_[0]);
            }

            AGCObject * iface() const noexcept {
                AGCObject * x = this->_iface();

                return (x->_has_null_vptr() ? nullptr : x);
            }

            /** Store interface pointer @p iface.
             *  We just want the vtable here
             **/
            void store_iface(const AGCObject * iface) {
                ::memcpy((void*)&(this->iface_[0]), (void*)iface, sizeof(AGCObject));
            }

        private:
            /** runtime interface for this object.
             *  We might prefer to declare this as AGCObject, but that's prohibited
             *  since AGCObject has abstract methods.
             *  Main downside of this form is it makes the data unintelligible to debugger.
             **/
            alignas(AGCObject) std::byte iface_[sizeof(AGCObject)];
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end ObjectTypeSlot.hpp */
