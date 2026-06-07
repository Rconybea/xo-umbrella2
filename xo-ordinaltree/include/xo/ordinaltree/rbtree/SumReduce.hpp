/**
 *  @file SumReduce.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/allocutil/ObjectVisitor.hpp"
#include <limits>

namespace xo {
    namespace tree {
        /* reduction for inverting the integral of a non-negative discrete function
         * computes sum of values for each subtree
         */
        template<typename Value>
        struct SumReduce {
            using value_type = Value;

            static constexpr bool is_monotonic() { return true; }

            value_type nil() const { return -std::numeric_limits<value_type>::infinity(); }
            value_type leaf(Value const & x) const {
                return x;
            } /*leaf*/

            value_type operator()(value_type reduced,
                                  Value const & x) const {
                /* sums tree values */
                if(std::isfinite(reduced)) {
                    return reduced + x;
                } else {
                    /* omit -oo reduced value from .nil() */
                    return x;
                }
            } /*operator()*/

            value_type combine(value_type const & x,
                               value_type const & y) const {
                /* omit -oo reduced value from .nil() */
                if(!std::isfinite(x))
                    return y;
                if(!std::isfinite(y))
                    return x;

                return x + y;
            } /*combine*/

            bool is_equal(value_type const & x, value_type const & y) const { return x == y; }
        }; /*SumReduce*/
    } /*namespace tree*/

    namespace gc {
        template <typename Value>
        class ObjectVisitor<xo::tree::SumReduce<Value>> {
        public:
            static_assert(std::is_empty_v<xo::tree::SumReduce<Value>>);

            /* trivial, since SumReduce<Value> is stateless */
            static void forward_children(xo::tree::SumReduce<Value> &, xo::gc::IAlloc *) {}
        };
    } /*namespace gc*/
}

/* end SumReduce.hpp */
