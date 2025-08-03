/* @file BooleanObj.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/Object.hpp"

namespace xo {
    namespace obj {
        /** @class BooleanObj
         *  @brief Boxed wrapper for a boolean value
         **/
        class BooleanObj : public Object {
        public:
            /** @return instance representing boolean with truth-value @p x **/
            static gp<BooleanObj> boolean_obj(bool x);
            static gp<BooleanObj> true_obj();
            static gp<BooleanObj> false_obj();

            bool value() const { return value_; }

            // inherited from Object..

            virtual std::size_t _shallow_size() const override;
            virtual Object * _shallow_copy() const override;
            virtual std::size_t _forward_children() override;

        private:
            explicit BooleanObj(bool x) : value_{x} {}

        private:
            bool value_;
        };
    }
}

/* end BooleanObj.hpp */
