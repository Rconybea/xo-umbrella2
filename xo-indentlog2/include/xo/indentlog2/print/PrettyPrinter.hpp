/** @file PrettyPrinter.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpConfig.hpp"
#include "LogBuffer.hpp"
#include "xo/arena/DArena.hpp"
#include "xo/arena/DArenaVector.hpp"

namespace xo {
    namespace print {
        class PrettyPrinter {
        public:
            using ArenaConfig = xo::mm::ArenaConfig;
            using DArena = xo::mm::DArena;
            using ScanStack = xo::mm::DArenaVector<uint32_t>;
            using PrintStack = xo::mm::DArenaVector<uint32_t>;

        public:
            explicit PrettyPrinter(const PpConfig & cfg);

            /** connect printer to output @p p_out **/
            void connect_output(LogBuffer * p_out);

            /** append c-string @p c_str **/
            void put_cstr(const char * c_str);

        private:
            /** count visible chars in range [lo, hi) **/
            uint32_t count_visible_chars(const char * lo,
                                         const char * hi) const;

            /** Available buffer space (in bytes).
             *  Can expand buffer up to size tk_buffer_.reserved()
             **/
            uint32_t available() const;

            /** Allocate token (always logically at scan_ix_) of size @p z **/
            void * alloc(uint32_t z);

            /** Expand to make space (at @ref scan_ix_) for token with size @p z **/
            void * expand_for(uint32_t z);

            /** adjust index values in [tk_src, tk_src+z)
             *  to new range [tk_dest, tk_dest+z)
             **/
            void reindex_stacks(const char * tk_dest,
                                const char * tk_src,
                                uint32_t z);

            /** print ready tokens.
             *  A token is ready if it's size if known,
             *  so we know whether it fits.
             **/
            void check_print_ready();

        private:
            /**
             *
             *   tk_buffer
             *   0                        limit_ix
             *   |                               |
             *   ........11111222334455555........
             *           ^                ^
             *    print_ix          scan_ix
             *
             * or wrapped:
             *
             *   tk_buffer
             *   0                       extent  limit_ix
             *   |                            |  |
             *   222334455555.............11111___
             *               ^            ^
             *         scan_ix     print_ix
             **/

            /** Configuration: max width, indent etc. **/
            PpConfig config_;
            /** Token buffer.
             *  Large enough to contain tokens up to @ref config_.right_margin_
             **/
            DArena tk_buffer_;
            /** Current buffer extent.
             *  Memory in [extent_, tk_buffer_.limit_) is unused;
             *  printer will wrap at extent_
             *  PpTokens have variable size, and since they're normal c++ objects,
             *  cannot wrap
             **/
            uint32_t extent_ = 0;
            /** append to @ref tk_buffer_ starting at this position **/
            uint32_t scan_ix_ = 0;
            /** print from @ref tk_buffer_ starting at this position **/
            uint32_t print_ix_ = 0;
            /** Counts visible characters from start-of-doc
             *  until @ref scan_ix_.
             **/
            uint32_t scan_viz_total_ = 0;
            /** Counts number of characters from start-of-doc
             *  until @ref scan_ix_.
             **/
            uint32_t scan_total_ = 0;
            /** Counts visible characeters from start-of-doc
             *  until @ref print_ix_
             *
             *  Invariant: @ref print_viz_total_ <= @ref scan_viz_total_
             **/
            uint32_t print_viz_total_ = 0;
            /** Counts number of characters from start-of-doc
             *  until @ref print_ix_
             *
             *  Invariant: @ref print_total_ <= @ref scan_total_
             **/
            uint32_t print_total_ = 0;
            /** Holds PpTokens for which size has *not* been computed.
             *  Each entry is an index into @ref tk_buffer_
             **/
            ScanStack scan_stack_;
            /** Holds PpTokens for which size *has* been computed,
             *  but which haven't yet been printed.
             **/
            PrintStack print_stack_;

            /** indent when pretty printer last attached to output **/
            uint32_t initial_indent_ = 0;

            /** output buffer.  Independently tracks lpos, visible chars **/
            LogBuffer * p_out_ = nullptr;
        };
    } /*namespace print*/
} /*namespace xo*/

/* end PrettyPrinter.hpp */
