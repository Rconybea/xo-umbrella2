/** @file callback_traits.hpp
 *
 *  Compile-time categories for CallbackSet's callback type.
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <xo/facet/obj.hpp>
#include <type_traits>
#include <functional>

namespace xo {
    namespace fn {
        /** true iff @tp T is std::function.  Rejected: it allocates from the
         *  heap, and its allocator-extended constructors were REMOVED in c++17
         *  (P0302R1), so it cannot be arena-backed even in principle.  It also
         *  hides captured state from the collector.
         **/
        template <typename T>
        struct is_std_function : std::false_type {};

        template <typename R, typename... Args>
        struct is_std_function<std::function<R (Args...)>> : std::true_type {};

        template <typename T>
        inline constexpr bool is_std_function_v = is_std_function<T>::value;

        /** true iff @tp T is a plain function pointer.  Safe by construction:
         *  a function pointer cannot capture, so it cannot hide a reference
         *  from the collector.
         **/
        template <typename T>
        inline constexpr bool is_plain_fn_ptr_v
            = (std::is_pointer_v<T>
               && std::is_function_v<std::remove_pointer_t<T>>);

        /** callback that participates in garbage collection.
         *
         *  A faceted callback can be pivoted to obj<AGCObject> at runtime via
         *  FacetRegistry, so a CallbackSet over it can be traced.
         *  is_fomo_v lives in xo-facet: it is a property of the object model,
         *  not of callbacks.
         **/
        template <typename Callback>
        concept fomo_callback_concept = xo::facet::is_fomo_v<Callback>;

        /** callback that does not, and cannot, hold traced references **/
        template <typename Callback>
        concept plain_callback_concept = is_plain_fn_ptr_v<Callback>;

        /** the callback types CallbackSet accepts.
         *
         *  Deliberately excludes std::function and closure types (lambdas):
         *  both can capture an obj<AFacet> that the collector cannot see, and
         *  neither can be allocated from an arena.  A lambda passed as a
         *  transient argument to invoke_each() is fine -- it is never stored.
         **/
        template <typename Callback>
        concept callback2_concept = (fomo_callback_concept<Callback>
                                     || plain_callback_concept<Callback>);
    } /*namespace fn*/
} /*namespace xo*/

/* end callback_traits.hpp */
