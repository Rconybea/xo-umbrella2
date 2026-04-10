/** @file DPrimitive.hpp
 *
 *  @author Roland Conybeare, Jan 2025
 **/

#pragma once

#include "RuntimeContext.hpp"
#include <xo/type/Type.hpp>
#include <xo/object2/DArray.hpp>
#include <xo/alloc2/GCObjectConversion.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/GCObject.hpp>
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
         *
         *  Distinction between type-constructor (@ref type_) and
         *  type-description (@ref fn_td_): type-constructor is narrower;
         *  it may lift into schematika type system some information about function
         *  behavior.  For example
         *  @pre
         *    cons :: (T x list<T>) -> list<T>
         *  @endpre
         *  but implementation has signature:
         *  @pre
         *    (obj<AGCobject> x obj<AGCObject,DList>) -> obj<AGCObject,DList>
         *  @endpre
         **/
        template <typename Fn>
        class Primitive {
        public:
            using FunctionPtrType = Fn;
            using Traits = detail::PmFnTraits<Fn>;
            using AGCObject = xo::mm::AGCObject;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;
            using DArray = xo::scm::DArray;
            using Reflect = xo::reflect::Reflect;
            using TypeDescr = xo::reflect::TypeDescr;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            /** @defgroup scm-primitive-ctors constructors **/
            ///@{

            Primitive(std::string_view name, obj<AType> type, Fn fn)
                : name_{name},
                  type_{type},
                  fn_td_{Reflect::require<Fn>()},
                  fn_{fn} {}

            static Primitive * _make(obj<AAllocator> mm, std::string_view name, Fn fn) {
                void * mem = mm.alloc_for<Primitive>();

                return new (mem) Primitive(name, obj<AType>(), fn);
            }

            static Primitive * _make(obj<AAllocator> mm,
                                     std::string_view name,
                                     obj<AType> type,
                                     Fn fn)
                {
                    void * mem = mm.alloc_for<Primitive>();

                    return new (mem) Primitive(name, type, fn);
                }

            ///@}
            /** @defgroup scm-primitive-methods general methods **/
            ///@{

            static constexpr std::int32_t n_args() noexcept { return Traits::n_args; }

            TypeDescr fn_td() const noexcept { return fn_td_; }
            std::string_view name() const noexcept { return name_; }
            bool is_nary() const noexcept { return false; }

            obj<AGCObject> apply_nocheck(obj<ARuntimeContext> rcx, const DArray * args) {
                return _apply_nocheck(rcx, args,
                                      std::make_index_sequence<Traits::n_args>{});
            }

            ///@}
            /** @defgroup scm-primitive-printable-facet **/
            ///@{

            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup scm-primitive-gcobject-facet **/
            ///@{
            Primitive * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
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

                obj<AAllocator> mm = rcx.allocator();

                R result
                    = fn_(rcx,
                          GCObjectConversion<typename Traits::template arg_type<Is>>::from_gco(mm, args->at(Is))... );

                return GCObjectConversion<R>::to_gco(mm, result);
            }

        private:
            /** name of this primitive **/
            std::string_view name_;

            /** primitive type-constructor. These are non-trivial to establish;
             *  not convenient to establish in ctor
             **/
            obj<AType> type_;

            /** type description for implementation function
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
        Primitive<Fn> *
        Primitive<Fn>::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept {
            return gc.std_move_for(this);
        }

        template <typename Fn>
        void
        Primitive<Fn>::visit_gco_children(xo::mm::VisitReason reason,
                                          obj<AGCObjectVisitor> gc) noexcept {
            gc.visit_poly_child(reason, &type_);
            //{
            //    auto e = type_.to_facet<AGCObject>();  // FacetRegistry dep
            //    gc.forward_inplace(e.iface(), (void **)&(type_.data_));
            //}
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DPrimitive.hpp */
