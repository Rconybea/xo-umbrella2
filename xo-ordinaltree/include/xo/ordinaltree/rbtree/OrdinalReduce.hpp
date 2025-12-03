/** @file OrdinalReduce.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include <cstdint>

#pragma once

namespace xo {
    namespace tree {

        /* just counts #of distinct values;
         * redundant,  same as detail::Node<>::size_.
         * providing for completeness' sake
         */
        template <typename Value>
        class OrdinalReduce {
        public:
            using value_type = std::size_t;

        public:
            static constexpr bool is_monotonic() { return true; }

            value_type nil() const { return 0; }

            value_type leaf(Value const & /*x*/) const {
                return 1;
            } /*leaf*/

            value_type operator()(value_type acc,
                                  Value const & /*x*/) const {
                /* counts #of values */
                return acc + 1;
            }

            value_type combine(value_type x, value_type y) const { return x + y; }
            bool is_equal(value_type x, value_type y) const { return x == y; }
        }; /*OrdinalReduce*/
    } /*namespace tree*/

    namespace gc {
        template <typename Value>
        class ObjectVisitor<xo::tree::OrdinalReduce<Value>> {
        public:
            static_assert(std::is_empty_v<xo::tree::OrdinalReduce<Value>>);

            static void forward_children(xo::tree::OrdinalReduce<Value> &, xo::gc::IAlloc *) {}
        };
    } /*namespace gc*/
} /*namespace xo*/

/* end OrdinalReduce.hpp */
