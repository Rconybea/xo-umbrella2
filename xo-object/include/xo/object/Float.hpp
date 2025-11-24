/** @file Float.hpp
 *
 * author: Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Number.hpp"

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
    }
}

/* end Float.hpp */
