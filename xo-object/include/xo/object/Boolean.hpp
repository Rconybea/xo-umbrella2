/* @file Boolean.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#pragma once

#include "ObjectConversion.hpp"
#include <xo/alloc/Object.hpp>
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xo::pp::xtag(..) */

namespace xo {
    namespace obj {
        /** @class Boolean
         *  @brief Boxed wrapper for a boolean value
         **/
        class Boolean : public Object {
        public:
            /** @return instance representing boolean with truth-value @p x **/
            static gp<Boolean> boolean_obj(bool x);
            static gp<Boolean> true_obj();
            static gp<Boolean> false_obj();
            static gp<Boolean> from(gp<Object> x) { return gp<Boolean>::from(x); }

            bool value() const { return value_; }

            // inherited from Object..

            virtual TaggedPtr self_tp() const final override;
            virtual void pretty(xo::pp::PpSink & sink) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * gc) const final override;
            virtual std::size_t _forward_children(gc::IAlloc * gc) final override;

        private:
            explicit Boolean(bool x) : value_{x} {}

        private:
            bool value_;
        };

        template <typename BoolType>
        struct ObjectConversion_Boolean {
            static gp<Object> to_object(gc::IAlloc * /*mm*/, BoolType x) {
                return Boolean::boolean_obj(x);
            }
            static BoolType from_object(gc::IAlloc *, gp<Object> x) {
                using xo::pp::tostr;
                using xo::pp::xtag;

                /* NB qualified, not a using-declaration.  The argument type
                 * gp<Object> lives in namespace xo, so ADL adds legacy
                 * xo::xtag to the candidate set -- and ADL is not suppressed
                 * by a using-decl, at block scope or otherwise.  Only a
                 * qualified call avoids the ambiguity.
                 */
                gp<Boolean> x_bool = Boolean::from(x);
                if (x_bool.get()) {
                    return x_bool->value();
                } else {
                    throw std::runtime_error(tostr("ObjectConversion_Boolean: x found where Boolean expected",
                                                   xtag("x", x)));
                }
            }
        };

        template <>
        struct ObjectConversion<bool> : public ObjectConversion_Boolean<bool> {};
    }
}

/* end Boolean.hpp */
