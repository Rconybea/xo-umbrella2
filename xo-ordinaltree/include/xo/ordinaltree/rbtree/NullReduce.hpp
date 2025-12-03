/** @file NullReduce.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/allocutil/ObjectVisitor.hpp"
#include <ostream>

namespace xo {
    namespace tree {
        struct null_reduce_value {};

        /* for null reduce,  just have it return empty struct;
         * otherwise breaks verification (e.g. verify_subtree_ok() below)
         */
        template<typename NodeValue>
        struct NullReduce {
            static constexpr bool is_null_reduce() { return true; }
            static constexpr bool is_monotonic() { return false; }

            /* data type for reduced values */
            using value_type = null_reduce_value;

            value_type nil() const { return value_type(); }
            value_type leaf(NodeValue const & /*x*/) const {
                return nil();
            }
            value_type operator()(value_type /*x*/,
                                  NodeValue const & /*value*/) const { return nil(); }
            value_type combine(value_type /*x*/,
                               value_type /*y*/) const { return nil(); }
            bool is_equal(value_type /*x*/, value_type /*y*/) const { return true; }
        }; /*NullReduce*/

        inline std::ostream & operator<<(std::ostream & os,
                                     null_reduce_value /*x*/)
        {
            os << "{}";
            return os;
        } /*operator<<*/
    } /*namespace tree*/

    namespace gc {
        template <typename Value>
        class ObjectVisitor<xo::tree::NullReduce<Value>> {
        public:
            static_assert(std::is_empty_v<xo::tree::NullReduce<Value>>);

            static void forward_children(xo::tree::NullReduce<Value> &, xo::gc::IAlloc *) {}
        };
    } /*namespace gc*/
} /*namespace xo*/

/* end NullReduce.hpp */
