/** @file JsonPrintState.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include <xo/reflect/TaggedPtr.hpp>
#include <xo/reflect/TypeDescr.hpp>
#include <iosfwd>

namespace xo {
    namespace json {
        class PrintJson;

        /** @brief one json print in progress
         *
         *  PrintJson's entry points (print, print_tp, print_obj) make one
         *  per top-level value.  A JsonPrinter receives it, and writes and
         *  recurses only through it: what a print needs to know about
         *  itself travels here, in plain sight, rather than in the printer
         *  or the printer table.
         *
         *  A printer must not call an entry point to print a child: that
         *  starts a print within a print.  See
         *  .xo-backlog/xo-printjson/issues/02.
         **/
        class JsonPrintState {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** a print of json on @p p_os, through @p pjson's printers **/
            JsonPrintState(PrintJson const * pjson, std::ostream * p_os);

            JsonPrintState(JsonPrintState const &) = delete;
            JsonPrintState & operator=(JsonPrintState const &) = delete;

            /** where json goes **/
            std::ostream * p_os() const { return p_os_; }

            /** true iff a printer was provided for @p td (PrintJson::provide_printer) **/
            bool has_printer(TypeDescr td) const;

            /** write json for @p tp: by its type's printer, if one was
             *  provided, else by the generic behaviour for its metatype
             *  (pointer, vector, struct).  The one way a printer recurses
             **/
            void print(TaggedPtr tp);

        private:
            /** the printer table **/
            PrintJson const * pjson_ = nullptr;
            /** where json goes **/
            std::ostream * p_os_ = nullptr;
        }; /*JsonPrintState*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonPrintState.hpp */
