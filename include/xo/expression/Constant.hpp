/** @file Constant.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "ConstantInterface.hpp"
#include <type_traits>

namespace xo {
    namespace ast {
        /** @class Constant
         *  @brief syntax for a literal constant.
         *
         *  Require:
         *  1. T must be a POD type (plain old data)
         *     We need this to be true so that we can generate
         *     code for constructing a T instance by memcopying
         *     @ref value_
         *
         *  @tp T type of captured literal.
         **/
        template <typename T>
        class Constant : public ConstantInterface {
        public:
            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** create constant expression representing literal value x **/
            static ref::rp<Constant> make(const T & x) {
                TypeDescr x_valuetype = Reflect::require<T>();

                return new Constant(x_valuetype, x);
            }

            const T & value() const { return value_; }

            // ----- ConstantInterface -----

            virtual TypeDescr value_td() const override { return value_td_; }
            virtual TaggedPtr value_tp() const override {
                /* note: idk why,  but need to spell this out in two steps with gcc 13.2 */
                const void * erased_cptr = &value_;
                void * erased_ptr = const_cast<void*>(erased_cptr);

                return TaggedPtr(value_td_, erased_ptr);
            }

            // ----- Expression -----

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                visitor_fn(this);
                return 1;
            }

            virtual void display(std::ostream & os) const override {
                os << "<Constant"
                   << xtag("type", value_td_->short_name())
                   << xtag("value", value_)
                   << ">";
            }

        private:
            explicit Constant(TypeDescr x_type, const T & x)
                : ConstantInterface(exprtype::constant, x_type),
                  value_td_{Reflect::require<T>()},
                  value_(x)
                {
                    static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
                }

        private:
            /** type description for T **/
            TypeDescr value_td_;
            /** value of this constant **/
            T value_;
        }; /*Constant*/

        template <typename T>
        ref::rp<Constant<std::remove_reference_t<T>>>
        make_constant(const T & x) {
            return Constant<T>::make(x);
        }

    } /*namespace ast*/
} /*namespace xo*/

/** end Constant.hpp **/
