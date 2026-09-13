/* @file JsonPrinter.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "JsonPrinter.hpp"
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/reflect/SelfTagging.hpp>
#include <xo/reflect/TypeDrivenMap.hpp>
#include <iostream>
#include <memory>
#include <type_traits>

namespace xo {
    namespace json {
        class PrintJson : public reflect::SelfTagging {
        public:
            using AReflectable = xo::reflect::AReflectable;
            using Reflect = xo::reflect::Reflect;
            using TypeDrivenMap = xo::reflect::TypeDrivenMap<std::unique_ptr<JsonPrinter>>;
            using SelfTagging = xo::reflect::SelfTagging;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TaggedRcptr = xo::reflect::TaggedRcptr;
            using TypeDescr = xo::reflect::TypeDescr;
            using TypeId = xo::reflect::TypeId;

        public:
            PrintJson();
            ~PrintJson() = default;

            template<typename T>
            void print(T const & x_arg, std::ostream * p_os) const {
                T * x = const_cast<T *>(&x_arg);

                this->print_tp(Reflect::make_tp(x), p_os);
            } /*print*/

            /* print object tp on stream *p_os, in JSON format;
             */
            void print_tp(TaggedPtr tp, std::ostream * p_os) const;

            /* convenience -- shorthand for
             *   .print(obj->self_tp(), p_os)
             */
            void print_obj(rp<SelfTagging> const & obj, std::ostream * p_os) const;

            /** print faceted object pointer @p x on @p p_os, in JSON format.
             *
             *  Templated on @tparam AFacet rather than fixed to AReflectable,
             *  because a caller holds whatever facet it was already using --
             *  obj<AGCObject> for an xo-object2 member, obj<APrintable> for a
             *  DObjectHandle.  Requiring obj<AReflectable> would make every
             *  caller rotate through FacetRegistry by hand, which is the work
             *  this is meant to remove.
             *
             *  THROWS if any representation reached has not implemented
             *  AReflectable, leaving a truncated document behind.  Call
             *  @ref validate_obj first for all-or-nothing.
             **/
            template <typename AFacet, typename DRepr>
            void print_obj(xo::facet::obj<AFacet, DRepr> x, std::ostream * p_os) const {
                if constexpr (std::is_same_v<AFacet, AReflectable>) {
                    /* fast path: x already carries the capability -- its iface
                     * IS an AReflectable implementation -- so ask it directly,
                     * with no FacetRegistry probe and no pointer hop.
                     *
                     * Guarded: an EMPTY obj<AReflectable> has IReflectable_Any
                     * for its iface, whose self_tp() terminates.  Falling
                     * through renders it as {}, matching the generic path.
                     */
                    if (x.data())
                        return this->print_tp(x.self_tp(), p_os);
                }

                this->print_tp(Reflect::make_tp(&x), p_os);
            } /*print_obj*/

            /** Walk everything @p tp reaches WITHOUT printing, throwing on the
             *  same condition @ref print_tp would.
             *
             *  Exists because that throw happens mid-traversal, so a consumer
             *  is left holding however much was already written.  Validate
             *  first, then print, for all-or-nothing.
             *
             *  Two properties, neither visible at the call site:
             *  - it costs a SECOND full traversal
             *  - it is only meaningful because traversal is synchronous;
             *    nothing may mutate the graph between the two passes
             *
             *  It does NOT make printing total.  A CYCLIC graph defeats both
             *  passes alike -- see .xo-backlog/xo-printjson/issues/02.
             **/
            void validate_tp(TaggedPtr tp) const;

            /** @ref validate_tp for a faceted object pointer; same fast path
             *  and same empty-object handling as @ref print_obj.
             **/
            template <typename AFacet, typename DRepr>
            void validate_obj(xo::facet::obj<AFacet, DRepr> x) const {
                if constexpr (std::is_same_v<AFacet, AReflectable>) {
                    if (x.data())
                        return this->validate_tp(x.self_tp());
                }

                this->validate_tp(Reflect::make_tp(&x));
            } /*validate_obj*/

            void provide_printer(TypeId id, std::unique_ptr<JsonPrinter> p) {
                *(printer_map_.require(id)) = std::move(p);
            }

            void provide_printer(TypeDescr td, std::unique_ptr<JsonPrinter> p) {
                this->provide_printer(td->id(), std::move(p));
            }

            /* write json representation for tp on *p_os */
            void print_aux(TaggedPtr tp, std::ostream * p_os) const;

            // ----- inherited from SelfTagging -----

            virtual TaggedRcptr self_tp();

        private:
            /* provide printers for common basic types */
            void provide_std_printers();

        private:
            /* map contains specialized printers for specific c++ types */
            TypeDrivenMap printer_map_;
        }; /*PrintJson*/

        /* Using singleton here to collect type-specific json printers,
         * collected during program initialization.
         *
         * Could relabel as PrintJsonInitContext if desired
         */
        class PrintJsonSingleton {
        public:
            static rp<PrintJson> instance();

        private:
            /* we don't need this to be stored as pointer.
             * memory burned if unused will be one empty std::vector<>
             */
            static rp<PrintJson> s_instance;
        }; /*PrintJsonSingleton*/

    } /*namespace json*/

#ifdef NOT_USING
    namespace print {
        using PrintJson = xo::json::PrintJson;

        /* stream inserter for printing a T-instance in json format */
        template<typename T>
        class jsonp_impl {
        public:
            jsonp_impl(T const & x, PrintJson const * pjson) : value_(x), pjson_{pjson} {}
            //jsonp_impl(T const & x, PrintJson const * pjson) : value_{x}, pjson_{pjson} {}
            //jsonp_impl(T && x, PrintJson const * pjson) : value_(std::move(x)), pjson_{pjson} {}

            void print(std::ostream & os) const {
                using xo::reflect::Reflect;

                this->pjson_->print_tp(Reflect::make_tp(&value_), &os);
            } /*print*/

        private:
            /* value, to be printed, in json format */
            T value_;
            /* json printer (bc we don't care for singletons) */
            PrintJson const * pjson_ = nullptr;
        }; /*jsonp_impl*/

        template<typename T>
        inline
        std::ostream & operator<<(std::ostream & os, jsonp_impl<T> const & x) {
            x.print(os);
            return os;
        } /*operator<<*/

        /* writing out std::forward<T> behavior for completeness' sake:
         *
         * 1. call jsonp(x) with rvalue std::string x, then:
         *    - T will be deduced to [std::string]
         *        (in particular: _not_ std::string &, std::string const &, std::string &&)
         *    - rvalue std::string passed to jsonp_impl ctor
         *
         * 2a. call jsonp(x) with std::string & x, then:
         *    - T deduced to [std::string &]
         *    - std::string & passed to jsonp_impl ctor
         *
         * 2b. call jsonp(x) with std::string const & x, then:
         *    - T deduced to [std::string const &]
         *    - std::string const & passed to jsonp_impl ctor
         */
        template<typename T>
        auto jsonp(T && x, PrintJson const * pjson) {
            return jsonp_impl<T>(std::forward<T>(x), pjson);
        } /*jsonp*/
    } /*namespace print*/
#endif
} /*namespace xo*/

/* end JsonPrinter.hpp */
