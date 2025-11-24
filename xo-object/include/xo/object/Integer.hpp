/* @file Integer.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#pragma once

#include "Number.hpp"

namespace xo {
    namespace obj {
        class Integer : public Number  {
        public:
            using IAlloc = xo::gc::IAlloc;
            using int_type = long long;

        public:
            Integer() = default;
            explicit Integer(int_type x);

            /** create instance holding integer value @p x **/
            static gp<Integer> make(IAlloc * mm, int_type x);
            /** downcast from @p x iff x is actually an Integer.  Otherwise nullptr **/
            static gp<Integer> from(gp<Object> x);

            int_type value() const { return value_; }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * gc) const final override;
            virtual std::size_t _forward_children(gc::IAlloc * gc) final override;

        private:
            int_type value_ = 0;
        };
    } /*namespace obj*/
} /*namespace xo*/

/* end Integer.hpp */
