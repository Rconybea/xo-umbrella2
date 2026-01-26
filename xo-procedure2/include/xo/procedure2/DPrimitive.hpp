/** @file DPrimitive.hpp
 *
 *  @author Roland Conybeare, Jan 2025
 **/

#pragma once

#include "RuntimeContext.hpp"
#include <xo/object2/DArray.hpp>
#include <xo/gc/GCObjectConversion.hpp>
#include <xo/gc/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/FacetRegistry.hpp>

#include <tuple>
#include <type_traits>
#include <utility>
#include <string_view>

namespace xo {
    namespace scm {
        namespace detail {
            /** @brief Extract return type and argument types from a function type.
             *
             *  Primary template (undefined) - specializations handle specific cases
             **/
            template <typename Fn>
            struct PmFnTraits;

            /** specialization for function pointers **/
            template <typename R, typename... Args>
            struct PmFnTraits<R(*)(obj<ARuntimeContext> rcx, Args...)> {
                /** function return type **/
                using return_type = R;
                /** tuple type for function arguments (except for runtime context) **/
                using args_tuple = std::tuple<Args...>;
                /** number of arguments (except for runtime context) **/
                static constexpr std::size_t n_args = sizeof...(Args);

                /** arg_type<i> is the type of the i'th argument to Fn.
                 *  (starting from argument after runtime context)
                 **/
                template <std::size_t I>
                using arg_type = std::tuple_element_t<I, args_tuple>;
            };

            /** specialization for function references **/
            template <typename R, typename... Args>
            struct PmFnTraits<R(&)(obj<ARuntimeContext>, Args...)> : PmFnTraits<R(*)(obj<ARuntimeContext>, Args...)> {};

            /** specialization for plain function types **/
            template <typename R, typename... Args>
            struct PmFnTraits<R(obj<ARuntimeContext>, Args...)> : PmFnTraits<R(*)(obj<ARuntimeContext>, Args...)> {};
        }

        /** @brief Schematika primitive with fixed number of arguments
         **/
        template <typename Fn>
        class Primitive {
        public:
            using AAllocator = xo::mm::AAllocator;
            using AGCObject = xo::mm::AGCObject;
            using DArray = xo::scm::DArray;
            using Traits = detail::PmFnTraits<Fn>;

        public:
            Primitive(std::string_view name, Fn fn) : name_{name}, fn_{fn} {}

            bool is_nary() const noexcept { return false; }
            static constexpr std::int32_t n_args() noexcept { return Traits::n_args; }

            obj<AGCObject> apply_nocheck(obj<ARuntimeContext> rcx, const DArray * args) {
                return _apply_nocheck(rcx, args,
                                      std::make_index_sequence<Traits::n_args>{});
            }

        private:
            template <std::size_t... Is>
            obj<AGCObject> _apply_nocheck(obj<ARuntimeContext> rcx,
                                          const DArray * args,
                                          std::index_sequence<Is...>)
            {
                using xo::facet::FacetRegistry;

                using R = typename Traits::return_type;

                assert(args);
                assert(args->size() > 0);

                obj<AAllocator> mm = rcx.allocator();

                R result
                    = fn_(rcx,
                          GCObjectConversion<typename Traits::template arg_type<Is>>::from_gco(mm, args->at(Is))... );

                return GCObjectConversion<R>::to_gco(mm, result);
            }

        private:
            /** name of this primitive **/
            std::string_view name_;
            /** function implementation **/
            Fn fn_;
        }; /*Primitive*/

    } /*namespace scm*/
} /*namespace xo*/

/* end DPrimitive.hpp */
