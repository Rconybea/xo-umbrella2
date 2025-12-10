/** @file AFacet.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <concepts>
#include <type_traits>

namespace xo {
    namespace facet {
        namespace detail {
            struct PlaceholderAbstractInterface {
                virtual double foo(void * data) const = 0;
            };

            static_assert(sizeof(PlaceholderAbstractInterface) == sizeof(void*));
        }

        /** Concept: abstract interface requirements
         *  Use: when inheriting an abstract interface
         *  (see also valid_abstract_interface() below)
         **/
        template <typename T>
        concept abstract_facet = requires {
            std::is_abstract_v<T>,
            std::is_polymorphic_v<T>;
            /** require no state, just a single vtable pointer **/
            sizeof(T) == sizeof(detail::PlaceholderAbstractInterface);
            !std::has_virtual_destructor_v<T>;
            std::is_trivially_destructible_v<T>;
        };

        /** Use: when defining an abstract facet AMyFacet
         *
         *    struct AMyFacet {
         *        virtual void foo(void * data) const = 0;
         *    };
         *
         *    static_assert(valid_abstract_facet<AMyFacet>());
         *
         **/
        template <typename T>
        consteval bool valid_abstract_facet()
        {
            static_assert
                (std::is_abstract_v<T>,
                 "Abstract facet is expected to have all-abstract methods");
            static_assert
                (std::is_polymorphic_v<T>,
                 "Abstract facet is expected to have vtable");
            static_assert
                (sizeof(T) == sizeof(detail::PlaceholderAbstractInterface),
                 "Abstract facet is expected to have no state except for a single vtable pointer");
            static_assert
                (!std::has_virtual_destructor_v<T>,
                 "Abstract facet does not benefit from virtual dtor since no state");
            static_assert
                (std::is_trivially_destructible_v<T>,
                 "Abstract facet expected to have trivial dtor since no state");
            return true;
        };

    } /*namespace facet*/
} /*namespace xo*/

/* end AFacet.hpp **/
