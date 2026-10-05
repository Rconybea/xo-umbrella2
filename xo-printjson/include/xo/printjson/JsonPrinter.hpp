/* @file JsonPrinter.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/reflect/TypeDrivenMap.hpp>
#include "JsonPrintState.hpp"
// #include <memory>
#include <iostream>

namespace xo {
    namespace json {
        /** @brief json printer for one c++ type, provided to a PrintJson
         *  (PrintJson::provide_printer)
         **/
        class JsonPrinter {
        public:
            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;
            using TypeId = xo::reflect::TypeId;

        public:
            virtual ~JsonPrinter() = default;

            /** write json for @p tp on @p state's output; print a child
             *  only via @p state.print()
             **/
            virtual void print_json(TaggedPtr tp,
                                    JsonPrintState & state) const = 0;

            /** true iff print_json writes a json object, opened with
             *  JsonPrintState::open_object (or writes null).  Such a value
             *  takes part in identity: JsonPrintState::print checks for it
             *  BEFORE calling print_json, so print_json never sees a value
             *  printed already.  A printer of scalars or arrays says false
             **/
            virtual bool prints_object() const { return true; }

            void report_internal_type_consistency_error(TypeDescr td1,
                                                        TypeDescr td2,
                                                        JsonPrintState & state) const;

            /* convenience method for derived printers.
             * retrieves contents of tp as a T*,  complains to state's output if that fails.
             *
             * (Failure would occur if printer for type T was instead installed
             *  for some unrelated type U)
             */
            template<typename T>
            T * check_recover_native(TaggedPtr tp, JsonPrintState & state) const {
                T * x = tp.recover_native<T>();

                if (!x) {
                    this->report_internal_type_consistency_error(Reflect::require<T>(),
                                                                 tp.td(),
                                                                 state);
                }

                return x;
            } /*check_recover_native*/
        }; /*JsonPrinter*/

        /* AsStringJsonPrinter<T>
         * prints a T-instance by using operator<< and surrounding in quotes.
         *
         * e.g:
         *   T & x = ..;
         *
         *   *state.p_os() << "\"" << x << "\""
         *
         */
        template<typename T>
        class AsStringJsonPrinter : public JsonPrinter {
        public:
            virtual bool prints_object() const override { return false; }

            virtual void print_json(TaggedPtr tp,
                                    JsonPrintState & state) const override
                {
                    T * x = this->check_recover_native<T>(tp, state);

                    if(x) {
                        *state.p_os() << "\"" << *x << "\"";
                    }
                } /*print_json*/
        }; /*AsStringJsonPrinter*/
    } /*namespace json*/
} /*namespace xo*/


/* end JsonPrinter.hpp */
