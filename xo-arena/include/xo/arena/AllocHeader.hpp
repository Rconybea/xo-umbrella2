/** @file AllocHeader.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/ppsink/Prettifier.hpp>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace xo {
    namespace pp { class PpSink; }

    namespace mm {
        /** @brief per-alloc header
         *
         *  Appears immediately before each allocation when
         *  ArenaConfig.store_header_flag_ is set.
         *
         *  See AllocInfo.hpp for encoding of @ref repr_
         **/
        struct AllocHeader {
            using repr_type = std::uintptr_t;
            using size_type = std::size_t;
            using PpSink = xo::pp::PpSink;

            explicit AllocHeader(repr_type x) : repr_{x} {}

            /** pretty print instance to @p sink.
             *
             *  Renders the packed word only.  Decoding it into type, age and
             *  size needs the AllocHeaderConfig that wrote it -- the widths
             *  are configurable, so the header cannot read itself.
             **/
            void pretty(PpSink & sink) const;

            repr_type repr_;
        };

        static_assert(sizeof(AllocHeader) == sizeof(AllocHeader::repr_type));
        static_assert(std::is_standard_layout_v<AllocHeader>);
    } /*namespace mm*/

    namespace pp {
        /** pretty-print for AllocHeader **/
        template <>
        struct Prettifier<xo::mm::AllocHeader> {
            static void print(PpSink & sink, const xo::mm::AllocHeader & x) {
                x.pretty(sink);
            }
        };
    } /*namespace pp*/
}

/* end AllocHeader.hpp */
