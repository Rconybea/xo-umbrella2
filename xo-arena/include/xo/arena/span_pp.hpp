/** @file span_pp.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  ppsink-native structured printing for xo::mm::span: Prettifier<span<CharT>>.
 *
 *  This is the ppsink counterpart to span_ppdetail.hpp (which supplies the
 *  legacy ostream operator<<).  A ppsink-focused subsystem that wants to
 *  pretty-print a span -- e.g. via sink.pp(x) or xo::pp::xtag("s", x) --
 *  includes this header; it renders straight into a PpSink and stays
 *  ostream-free (addr via snprintf, size via std::to_chars).
 *
 *  Include span.hpp alone for the span type with no printing at all.
 **/

#pragma once

#include "xo/arena/span.hpp"
#include <xo/ppsink/quoted.hpp>
#include <xo/ppsink/tag.hpp>       /* xo::pp::{Prettifier, xtag, PpSink, pretty} */
#include <string_view>
#include <charconv>
#include <cstdio>

namespace xo::pp {
    /** @brief structured pretty-printing for xo::mm::span into a PpSink.
     *
     *  Renders as  @code <span :addr 0x.. :size N :text CONTENT> @endcode
     *  (mirrors the legacy span ostream summary), with each field a tag so
     *  it participates in pretty-print line breaking.  ostream-free.
     **/
    template <typename CharT>
    struct Prettifier<xo::mm::span<CharT>> {
        static void print(PpSink & sink, const xo::mm::span<CharT> & x) {
            char addr_buf[2 + 2 * sizeof(void *) + 1];   /* "0x" + hex digits + NUL */
            std::snprintf(addr_buf, sizeof(addr_buf), "%p", (const void *)x.lo());

            char size_buf[24];                           /* holds any uint64 */
            auto [size_end, ec] = std::to_chars(size_buf, size_buf + sizeof(size_buf),
                                                x.size());
            (void)ec;   /* cannot fail: buf is large enough */

            sink.put("<span");
            sink.begin(2);
            sink.pp(tag("addr", std::string_view(addr_buf)));
            sink.split(1,2);
            sink.pp(tag("size", std::string_view(size_buf,
                                                 static_cast<std::size_t>(size_end - size_buf))));
            sink.split(1,2);
            sink.pp(tag("text", unq(x.to_string_view())));
            sink.put(">");
            sink.end();
        }
    };
} /*namespace xo::pp*/

/* end span_pp.hpp */
