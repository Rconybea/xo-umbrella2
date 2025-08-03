/* @file Integer.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Number.hpp"

namespace xo {
    namespace obj {
        class Integer : public Number  {
        public:
            using int_type = long long;

        public:
            Integer() = default;
            explicit Integer(int_type x);

            static gp<Integer> make(int_type x);

            // inherited from Object..
            virtual std::size_t _shallow_size() const override;
            virtual Object * _shallow_copy() const override;
            virtual std::size_t _forward_children() override;

        private:
            int_type value_ = 0;
        };
    } /*namespace obj*/
} /*namespace xo*/

/* end Integer.hpp */
