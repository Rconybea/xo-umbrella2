/** @file type_keys.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include <xo/reflect/TypeDescr.hpp>
#include <ostream>
#include <string_view>

namespace xo {
    namespace json {
        /** @brief the json keys naming a type, forwarding its TypeDescr:
         *
         *    "_canonical_type_": td->canonical_name(), "_short_type_": td->short_name()
         *
         *  _canonical_type_ is unique: the key into a type -> source map
         *  (.xo-backlog/xo-websock/issues/12).  _short_type_ is for display.
         *
         *  Use in a JsonPrinter, after the object's "_name_":
         *
         *    *p_os << "{" << quot("_name_") << ": " << quot("Foo")
         *          << ", " << type_keys(tp.td());
         *
         *  Holds views: use it within the expression that creates it.
         **/
        struct type_keys {
            explicit type_keys(reflect::TypeDescr td)
                : canonical_{td->canonical_name()}, short_{td->short_name()} {}
            /** for a type with no TypeDescr -- a C++ reference **/
            type_keys(std::string_view canonical, std::string_view short_name)
                : canonical_{canonical}, short_{short_name} {}

            std::string_view canonical_;
            std::string_view short_;
        };

        std::ostream & operator<<(std::ostream & os, type_keys const & k);
    } /*namespace json*/
} /*namespace xo*/

/* end type_keys.hpp */
