/* @file JsonPrinter.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "JsonPrinter.hpp"
#include "xo/reflect/TypeDrivenMap.hpp"
#include "xo/reflect/SelfTagging.hpp"
#include <memory>
#include <iostream>

namespace xo {
    namespace json {
        class PrintJson : public reflect::SelfTagging {
        public:
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
            void print_obj(ref::rp<SelfTagging> const & obj, std::ostream * p_os) const;

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
            static ref::rp<PrintJson> instance();

        private:
            /* we don't need this to be stored as pointer.
             * memory burned if unused will be one empty std::vector<>
             */
            static ref::rp<PrintJson> s_instance;
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
