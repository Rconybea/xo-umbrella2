/* @file Boolean.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/Object.hpp"

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

            bool value() const { return value_; }

            // inherited from Object..

            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * gc) const final override;
            virtual std::size_t _forward_children(gc::GC * gc) final override;

        private:
            explicit Boolean(bool x) : value_{x} {}

        private:
            bool value_;
        };
    }
}

/* end Boolean.hpp */
