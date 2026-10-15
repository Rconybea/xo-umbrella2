/** @file json_quoted.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  json_quot(s): write @p s to an ostream as a JSON string literal --
 *  quoted, and escaped in JSON's vocabulary (@c \u00NN, never @c \xNN).
 *
 *  Not xo::pp::quot(): that escapes per its sink's style, and @c os << quot(s)
 *  builds its sink with the DEFAULT style, i.e. display escaping.  This
 *  builds one with PpStyle::json().  See .xo-backlog/xo-printjson/issues/04.
 *
 *  A bridge for as long as printjson writes to std::ostream: once it writes
 *  into a json-styled sink (.xo-backlog/xo-printjson/issues/01), quot()
 *  through that sink is already right and this goes.
 **/

#pragma once

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <ostream>
#include <string_view>

namespace xo {
    namespace json {
        /** @brief a string to write as a JSON string literal **/
        class json_quot_impl {
        public:
            explicit constexpr json_quot_impl(std::string_view s) noexcept : s_{s} {}

            constexpr std::string_view value() const noexcept { return s_; }

        private:
            /** the string to write **/
            std::string_view s_;
        };

        /** write @p s quoted, with JSON escapes **/
        inline constexpr json_quot_impl
        json_quot(std::string_view s) noexcept {
            return json_quot_impl(s);
        }

        inline std::ostream &
        operator<<(std::ostream & os, const json_quot_impl & x) {
            xo::pp::FlatSink sink(xo::pp::PpStyle::json(), os.rdbuf());

            sink.put_with_escape(x.value(), true /*quote_flag*/);

            return os;
        }
    } /*namespace json*/
} /*namespace xo*/

/* end json_quoted.hpp */
