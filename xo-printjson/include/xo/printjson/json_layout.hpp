/** @file json_layout.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  Layout tokens for json written into a PpSink: where an object or array
 *  may break across lines.
 *
 *    write(json_open('{'), first, json_sep(), second, json_close('}'));
 *
 *  A group that fits the margin stays on one line, exactly as flat output
 *  (a FlatSink renders json_sep() as ", " and the open/close as the bare
 *  bracket).  One that does not breaks at every json_sep() -- one item per
 *  line, indented c_json_indent past the opening line -- and puts its
 *  closing bracket on its own line, back at the opening line's indent.
 *  See .xo-backlog/xo-printjson/issues/01
 **/

#pragma once

#include <xo/ppsink/Prettifier.hpp>
#include <cstdint>

namespace xo {
    namespace json {
        /** indent of a broken object's or array's items, past its opening line **/
        static constexpr std::int32_t c_json_indent = 2;

        /** opening bracket of an object or array, and the group it begins **/
        struct json_open {
            char bracket_;
        };

        /** separator between items: "," and a place to break **/
        struct json_sep {
            /** spaces when the group fits: 1 gives ", "; 0 gives "," **/
            std::uint32_t spaces_ = 1;
        };

        /** closing bracket, and the end of the group json_open began **/
        struct json_close {
            char bracket_;
        };
    } /*namespace json*/

    namespace pp {
        template <>
        struct Prettifier<xo::json::json_open> {
            using json_open = xo::json::json_open;

            static void print(PpSink & sink, json_open const & x) {
                using xo::json::c_json_indent;

                sink.put(std::string_view(&x.bracket_, 1));
                sink.begin(c_json_indent);
                sink.split(0, 0);
            }
        };

        template <>
        struct Prettifier<xo::json::json_sep> {
            using json_sep = xo::json::json_sep;

            static void print(PpSink & sink, json_sep const & x) {
                sink.put(",");
                sink.split(x.spaces_, 0);
            }
        };

        template <>
        struct Prettifier<xo::json::json_close> {
            using json_close = xo::json::json_close;

            static void print(PpSink & sink, json_close const & x) {
                using xo::json::c_json_indent;

                /* inside the group, so it breaks only when the group does */
                sink.split(0, -c_json_indent);
                sink.end();
                sink.put(std::string_view(&x.bracket_, 1));
            }
        };
    } /*namespace pp*/
} /*namespace xo*/

/* end json_layout.hpp */
