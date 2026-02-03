/** @file DPrimitive.hpp
 *
 *  @author Roland Conybeare, Jan 2025
 **/

#pragma once

#include "RuntimeContext.hpp"
#include <xo/object2/DArray.hpp>
#include <xo/gc/GCObjectConversion.hpp>
#include <xo/gc/GCObject.hpp>
#include <xo/reflect/Reflect.hpp>
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
            using Traits = detail::PmFnTraits<Fn>;

            using ACollector = xo::mm::ACollector;
            using AAllocator = xo::mm::AAllocator;
            using AGCObject = xo::mm::AGCObject;
            using DArray = xo::scm::DArray;
            using Reflect = xo::reflect::Reflect;
            using TypeDescr = xo::reflect::TypeDescr;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            Primitive(std::string_view name, Fn fn) : name_{name},
                                                      fn_td_{Reflect::require<Fn>()},
                                                      fn_{fn} {}

            TypeDescr fn_td() const noexcept { return fn_td_; }

            std::string_view name() const noexcept { return name_; }
            static constexpr std::int32_t n_args() noexcept { return Traits::n_args; }

            bool is_nary() const noexcept { return false; }

            obj<AGCObject> apply_nocheck(obj<ARuntimeContext> rcx, const DArray * args) {
                return _apply_nocheck(rcx, args,
                                      std::make_index_sequence<Traits::n_args>{});
            }

            /** @defgroup scm-primitive-printable-facet **/
            ///@{

            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup scm-primitive-gcobject-facet **/
            ///@{
            std::size_t shallow_size() const noexcept;
            Primitive * shallow_copy(obj<AAllocator> mm) const noexcept;
            std::size_t forward_children(obj<ACollector> gc) noexcept;
            ///@}

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

            /** type description for function
             *  Note that this type description will have additional first argument
             *  for obj<ARuntimeContext>
             **/
            TypeDescr fn_td_;

            /** function implementation **/
            Fn fn_;
        }; /*Primitive*/

        template <typename Fn>
        bool
        Primitive<Fn>::pretty(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct
                       (ppii,
                        "Primitive<Fn>",
                        refrtag("name", name_),
                        refrtag("td", fn_td_),
                        refrtag("fn", fn_));
        }

        template <typename Fn>
        std::size_t
        Primitive<Fn>::shallow_size() const noexcept {
            return sizeof(*this);
        }

        template <typename Fn>
        Primitive<Fn> *
        Primitive<Fn>::shallow_copy(obj<AAllocator> mm) const noexcept {
            void * mem = mm.alloc_copy((std::byte *)this);

            if (mem) {
                return new (mem) Primitive(*this);
            }

            return nullptr;
        }

        template <typename Fn>
        std::size_t
        Primitive<Fn>::forward_children(obj<ACollector>) noexcept {
            // Primitive holds no GC refs (just string_view + function pointer)
            return this->shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DPrimitive.hpp */
