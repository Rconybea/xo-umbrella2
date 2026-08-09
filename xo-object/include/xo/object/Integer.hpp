/* @file Integer.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#pragma once

#include "Number.hpp"
#include "ObjectConversion.hpp"
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xo::pp::xtag(..) */

namespace xo {
    namespace obj {
        class Integer : public Number  {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;
            using IAlloc = xo::gc::IAlloc;
            using int_type = long long;

        public:
            Integer() = default;
            explicit Integer(int_type x);

            /** create instance holding integer value @p x **/
            static gp<Integer> make(IAlloc * mm, int_type x);
            /** downcast from @p x iff x is actually an Integer.  Otherwise nullptr **/
            static gp<Integer> from(gp<IObject> x);

            int_type value() const { return value_; }

            void assign_value(int_type x) { value_ = x; }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void pretty(xo::pp::PpSink & sink) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * gc) const final override;
            virtual std::size_t _forward_children(gc::IAlloc * gc) final override;

        private:
            int_type value_ = 0;
        };

        template <typename IntType>
        struct ObjectConversion_Integer {
            static gp<Object> to_object(gc::IAlloc * mm, IntType x) {
                return new (MMPtr(mm)) Integer(x);
            }
            static IntType from_object(gc::IAlloc *, gp<Object> x) {
                /* NB qualified, not a using-declaration.  The argument type
                 * gp<Object> lives in namespace xo, so ADL adds legacy
                 * xo::xtag to the candidate set -- and ADL is not suppressed
                 * by a using-decl, at block scope or otherwise.  Only a
                 * qualified call avoids the ambiguity.
                 */
                gp<Integer> x_int = Integer::from(x);
                if (x_int.get()) {
                    return x_int->value();
                } else {
                    throw std::runtime_error(xo::pp::tostr("ObjectConversion_Integer: x found where Integer expected", xo::pp::xtag("x", x)));
                }
            }
        };

        template <>
        struct ObjectConversion<int64_t> : public ObjectConversion_Integer<int64_t> {};
        template <>
        struct ObjectConversion<int32_t> : public ObjectConversion_Integer<int32_t> {};
        template <>
        struct ObjectConversion<int16_t> : public ObjectConversion_Integer<int16_t> {};

    } /*namespace obj*/
} /*namespace xo*/

/* end Integer.hpp */
