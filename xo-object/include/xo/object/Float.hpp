/** @file Float.hpp
 *
 * author: Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Number.hpp"
#include "ObjectConversion.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
    namespace obj {
        class Float : public Number {
        public:
            using IAlloc = xo::gc::IAlloc;
            using float_type = double;

        public:
            Float() = default;
            explicit Float(float_type x);

            /** create instance holding floating-point value @p x **/
            static gp<Float> make(IAlloc * mm, float_type x);
            /** downcast from @p x iff x is actually a Float. Otherwise nullptr **/
            static gp<Float> from(gp<Object> x);

            float_type value() const { return value_; }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * gc) const final override;
            virtual std::size_t _forward_children(gc::IAlloc * gc) final override;

        private:
            float_type value_ = 0.0;
        };

        template <typename FloatType>
        struct ObjectConversion_Float {
            static gp<Object> to_object(gc::IAlloc * mm, FloatType x) {
                return new (MMPtr(mm)) Float(x);
            }
            static FloatType from_object(gc::IAlloc *, gp<Object> x) {
                gp<Float> x_int = Float::from(x);
                if (x_int.get()) {
                    return x_int->value();
                } else {
                    throw std::runtime_error(tostr("ObjectConversion_Float: x found where Float expected", xtag("x", x)));
                }
            }
        };

        template <>
        struct ObjectConversion<double> : public ObjectConversion_Float<double> {};
        template <>
        struct ObjectConversion<float> : public ObjectConversion_Float<float> {};
    }
}

/* end Float.hpp */
