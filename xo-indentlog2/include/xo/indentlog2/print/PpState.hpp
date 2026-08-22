/** @file PpState.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpConfig.hpp"
#include "xo/indentlog2/LogBuffer.hpp"
#include <xo/arena/DArena.hpp>
#include <xo/arena/DArenaVector.hpp>

namespace xo::pp {
    class PpStringToken; // see PpToken.hpp

    /**
     *  Use:
     *  @code
     *    // create pretty-printer
     *    PpState pps(PpConfig());
     *    LogBuffer logbuf(ArenaConfig(..));
     *    pps.connect_output(&logbuf);
     *
     *    // pretty-print foo(apple, banana).
     *    // If space-limited, split onto multiple lines like:
     *    //   foo(
     *    //     apple,
     *    //     banana)
     *    //
     *    pps.put_cstr("foo");
     *    pps.put_cstr("(");
     *
     *    pps.begin();
     *    pps.split();
     *
     *    pps.put_cstr("apple");
     *    pps.put_cstr(",");
     *
     *    pps.split();
     *
     *    pps.put_cstr("banana");
     *    pps.put_cstr(")");
     *
     *    pps.end();
     *  @endcode
     **/
    class PpState {
    public:
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
        using ArenaConfig = xo::mm::ArenaConfig;
        using DArena = xo::mm::DArena;
        using Span = LineState::Span; // = xo::mm::span<char>
        using ScanStack = xo::mm::DArenaVector<uint32_t>;
        using PrintStack = xo::mm::DArenaVector<uint32_t>;

    public:
        /** Create instance from configuration @p cfg.
         *  Allocates arena for @ref logbuf_
         **/
        explicit PpState(const PpLayoutConfig & layout);

        /** true iff printer currently has an open string
         *  (only one permitted at a time, since zero-copy)
         **/
        bool has_open_string() const { return current_open_string_ != nullptr; }

        /** visit mapped storage pools, call @p fn(pool) for each one **/
        void visit_pools(const MemorySizeVisitor & fn) const;

        /** connect printer to output @p p_out **/
        void connect_output(LogBuffer * p_out);

        /** append @p str to pretty printer as a single token;
         *  triggers printing when at top level.
         **/
        void put(std::string_view str);
        /** append @p c_str to pretty printer as a single token;
         *  triggers printing at top-level
         **/
        void put_cstr(const char * c_str);
        /** append @p str as a single token, with escapes expanded per
         *  xo::pp::Escape (see xo/ppsink/escape.hpp), surrounded by
         *  Escape::c_quote iff @p quote_flag.
         *
         *  Always exactly one token, so the pretty printer can never break a
         *  line inside the escaped string, or between a quote and its content.
         **/
        void put_with_escape(std::string_view str, bool quote_flag);
        /** begin a group; @p offset adds to the running indent until end().
         *  the no-arg form indents one nesting level (config indent_width).
         **/
        void begin(int32_t offset);
        void begin() { this->begin(layout_config_.indent_width()); }
        /** begin a group whose indent ORIGIN is the current output column,
         *  rather than the running indent.
         **/
        void begin_here(int32_t offset);
        void begin_here() { this->begin_here(0); }

    public:
        /** optional break.  Group fits: emit @p spaces spaces.
         *  Group breaks: newline + (running_indent + @p offset).
         **/
        void split(uint32_t spaces, int32_t offset);
        void split() { this->split(0, 0); }
        void split(uint32_t spaces) { this->split(spaces, 0); }
        /** forced break: always newline + (running_indent + @p offset),
         *  forcing every enclosing group to break.
         **/
        void newline(int32_t offset = 0);
        /** end a group (append @c k_end token); may trigger printing **/
        void end();

        /** Special case for assembling a string token incrementally
         *  (Relying on this for interaction w/ exogenous operator<<).
         *
         *  Similar to put() / put_cstr(), except that string length
         *  isn't determined yet.  Create string token, ensuring
         *  contiguous memory for at least @p min_z chars
         **/
        Span open_string(uint32_t min_z);
        /** Finalize a string token initiated previously by @ref open_string(). **/
        void commit_string(Span str);

    private:
        /** common implementation of begin() / begin_here() **/
        void _begin_aux(int32_t offset, bool align_here);

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
        /** Allocate at @ref scan_ix_. Helper for alloc() **/
        void * alloc_scan_aux(uint32_t z);

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

        /** write @p n spaces to the output (no newline) **/
        void write_spaces(uint32_t n);

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

        /** Configuration: layout-specific: max width, indent etc. **/
        PpLayoutConfig layout_config_;

        /** Configuration: logbuffer size / debug **/
        PpLogbufConfig logbuf_config_;

        /** Token buffer.
         *  Large enough to contain tokens up to @ref config_.right_margin_
         **/
        DArena tk_buffer_;
        /** PpTokens have variable size and each token must occupy
         *  contiguous memory.
         *
         *  Current buffer extent, when buffer is wrapped
         *  (with @ref scan_ix_ < @ref print_ix_).
         *  Established whenever buffer is an unwrapped state
         *  (with @ref print_ix_ <= scan_ix_) but next scanned token
         *  would be too big for remaining buffer space
         *  @c [scan_ix_, tk_buffer_.limit_).
         *
         *  Memory in [extent_, tk_buffer_.limit_) is unusable
         *  until either buffer unwraps (@ref print_ix_ resets to 0)
         *  or buffer fills and printer printer expands it.
         **/
        uint32_t extent_ = 0;
        /** append to @ref tk_buffer_ starting at this position **/
        uint32_t scan_ix_ = 0;
        /** print from @ref tk_buffer_ starting at this position **/
        uint32_t print_ix_ = 0;
        /** Counts number of visible characters from incoming string tokens
         *  since inception. Monotonically increasing.
         **/
        uint32_t scan_viz_total_ = 0;
        /** Counts number of characters from incoming string tokens
         *  since inception. Monotonically increasing.
         **/
        uint32_t scan_total_ = 0;
        /** Counts visible pretty-printed characters
         *  since inception, excluding whitespace inserted by
         *  pretty printer itself. Chases @ref scan_viz_total_
         *
         *  Invariant: @ref print_viz_total_ <= @ref scan_viz_total_
         **/
        uint32_t print_viz_total_ = 0;
        /** Counts pretty-printed characters since inception,
         *  excluding whitespace inserted by pretty printer itself.
         *  Chases @ref scan_total_.
         *
         *  Invariant: @ref print_total_ <= @ref scan_total_
         **/
        uint32_t print_total_ = 0;
        /** Indentifies k_begin PpTokens for which size has not yet been computed.
         *  Each entry is an index into @ref tk_buffer_.
         *  Size for k_begin tokens is unknown until pretty printer
         *  encounters matching k_end token.
         **/
        ScanStack scan_stack_;
        /** Holds PpTokens for which size *has* been computed,
         *  but which haven't yet been printed to @c *p_out_.
         **/
        PrintStack print_stack_;
        /** running indent while printing: sum of the offsets of the currently
         *  open groups (k_begin += offset, k_end -= offset).  A broken split
         *  indents to (print_indent_ + that split's own offset).
         **/
        int32_t print_indent_ = 0;

        /** if non-null: a string token initiated by @ref open_string().
         *  Actual size established when @ref commit_string() runs.
         *  At most one such token can exist.
         *  Used to support interaction with external operator<<
         **/
        PpStringToken * current_open_string_ = nullptr;

        /** output buffer.  Independently tracks lpos, visible chars **/
        LogBuffer * p_out_ = nullptr;
    };
} /*namespace xo::pp*/

/* end PpState.hpp */
