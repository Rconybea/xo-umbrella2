/** @file gc_allocator_traits.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <type_traits>
#include <memory>

namespace xo {
    namespace gc {
        /** Extended version of
         *    std::allocator_traits<Allocator>
         *  Introduces additional i/face methods
         *  for garbage-collector-enabled allocators
         *
         *  allocator A can identify itself as a copying collector:
         *  1. provide A::object_interface
         *       A::object_interface = xo::Object
         *  2. provide A::is_incremental_collector
         *       A::is_incremental_collector = std::true_type
         *     Collectible objects must:
         *     2a. inherit A::object_interface
         *     2b. implement A::object_interface::_shallow_size()
         *     2c. implement A::object_interface::_shallow_copy(alloc)
         *     2d. implement A::object_interface::_forward_children(alloc)
         **/
        template <typename Allocator>
        struct gc_allocator_traits : std::allocator_traits<Allocator> {
            using super = std::allocator_traits<Allocator>;
            using pointer = typename super::pointer;
            using value_type = typename super::value_type;

            // default: allocator A fallback to standard non-gc allocator behavior
            template <typename A, typename = void>
            struct is_incremental_collector : std::false_type {};

            // opt-in: A provides nested type 'is_incremental_collector':
            // struct A {
            //   using is_incremental_collector = std::true_type;
            // };
            template <typename A>
            struct is_incremental_collector<A, std::void_t<typename A::is_incremental_collector>> :
                A::is_incremental_collector {};

            // default: empty object interface.
            // classes that want to conditionally support GC
            // (e.g. see xo::tree::RedBlackTree, xo::tree::Node
            //       in xo-ordinal-tree)
            // can inherit
            //   gc_allocator_traits<Allocator>::template object_interface<Allocator>
            //
            template <typename A, typename = void>
            struct object_interface {};

            // specialization when A provides gc_object_interface
            template <typename A>
            struct object_interface<A, std::void_t<typename A::gc_object_interface>>
                : A::gc_object_interface {};

            /** true iff this allocator advertises itself as an incremental collector
             *  allocator will include:
             *
             *  struct GC {
             *    using is_incremental_collector = std::true_type;
             *  };
             **/
            static inline constexpr bool is_incremental_collector_v = is_incremental_collector<Allocator>::value;

        };
    } /*namespace gc*/
} /*namespace xo*/

/* end gc_allocator_traits.hpp */
