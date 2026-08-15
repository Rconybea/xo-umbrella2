/** @file LogBuffer.hpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#pragma once

#include "LineState.hpp"
#include <xo/arena/DArena.hpp>
#include <iosfwd>

namespace xo {
    using CharBuffer = xo::mm::DArena;

    /** @brief Arena-backed buffer storage for logging and pretty printing **/
    class LogBufferAdapter {
        // Originally adapted from xo::log_streambuf, with changes:
        // 1. Use DArena for memory.
        // 2. Accounting in separate class (LineState, q.v.).
        // 3. Explicit buffer pointers.
        // 4. Make streambuf dependency optional (see LogStreambuf)
        // 5. Dropping the CharT parameter, focus utf8
    public:
        using ArenaConfig = xo::mm::ArenaConfig;
        using DArena = xo::mm::DArena;
        using Span = LineState::Span; // = xo::mm::span<char>;
        using ConstSpan = LineState::ConstSpan; // = xo::mm::span<const char>;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
        using size_t = std::size_t;

    public:
        explicit LogBufferAdapter(DArena & buf, bool debug_flag);

        bool debug_flag() const { return debug_flag_; }
        size_t local_ppos() const { return lstate_.local_ppos(); }
        size_t lpos() const { return lstate_.lpos(); }
        size_t viz_lpos() const { return lstate_.viz_lpos(); }

        /** allocated buffer extent available to hold content (allocated + available) **/
        Span committed_span() const;
        /** used buffer extent **/
        Span used_span() const;
        /** available (contiguous) buffer extent **/
        Span available_span() const;

        /** visit mapped storage pools **/
        void visit_pools(const MemorySizeVisitor & fn) const;

        /** expand buffered character storage to at least @p new_z.
         *  @return true on success, false if oom
         **/
        bool expand_to(size_t new_z);

        /** write newline and @p indent spaces **/
        void newline_indent(uint32_t indent);
        /** write span @p x to buffer **/
        void write_span(ConstSpan x);

        /** attach (or detach, with nullptr) a streambuf that buffered
         *  output is drained to by @ref flush (hence by @ref reset_buffer).
         *  nullptr => pure buffer: accumulate, drain nothing (default).
         **/
        void set_dest_sbuf(std::streambuf * x) { dest_ = x; }

        /** drain the not-yet-flushed extent [@ref bpptr_, @ref pptr_) to
         *  @ref dest_ (if attached) and advance @ref bpptr_ to @ref pptr_.
         *  No-op when no dest_ is attached or nothing is pending.
         *  Safe to call at any time: LogBuffer writes are irrevocable, so
         *  drained bytes are never un-written.
         **/
        void flush();

        /** reset to nominal state, ready to receive output,
         *  but with no buffered content. Flushes any pending extent first
         *  (so drained output isn't lost), then resets @ref lstate_.
         **/
        void reset_buffer();

        /** require room for @p x chars **/
        bool _require_avail(uint32_t x);

        /** synchronize line accountant @ref lstate_ when dirty
         *  (because chars possibly added to @ref buf_v_).
         *  Also refresh @ref pptr_
         **/
        void _check_update_local_state(char * pptr);

    private:
        /** drain the completed line to @ref dest_ and rewind the buffer to
         *  @ref porigin_ for reuse.  Valid only at a line boundary (current
         *  line empty), where it needs no move: see @ref newline_indent.
         **/
        void reclaim_line();

    private:
        /** @defgroup LogBuffer-instance-vars **/
        ///@{

        /** buffer storage here **/
        DArena & buf_v_;
        /** checkpoint for realloc **/
        DArena::Checkpoint buf_ckp_;
        /** pinned origin of usable buffered memory.
         *  Unlike streambuf's pbase, this does NOT advance on flush:
         *  @ref lstate_ needs [porigin_, pptr_) to compute line position.
         **/
        char * porigin_ = nullptr;
        /** begin of the not-yet-flushed put area (mirrors @ref epptr_).
         *  [porigin_, bpptr_) already drained to @ref dest_;
         *  [bpptr_, pptr_) written but not yet drained.
         *  This pointer plays the role streambuf calls pbase.
         **/
        char * bpptr_ = nullptr;
        /** current write pointer **/
        char * pptr_ = nullptr;
        /** end of usable buffered memory (can be expanded) **/
        char * epptr_ = nullptr;
        /** line state computed incrementally from contents of @ref buf_v_ **/
        LineState lstate_;
        /** true to debug LogBuffer itself **/
        bool debug_flag_ = false;

        /** optional drain target; nullptr => pure buffer (see @ref set_dest_sbuf) **/
        std::streambuf * dest_ = nullptr;

        ///@}
    };

    /** Buffer for logging, owns its arena
     **/
    class LogBuffer : public LogBufferAdapter {
    public:
        /** Create instance using @p config for @ref buf_v_ **/
        LogBuffer(const ArenaConfig & config, bool debug_flag);

    private:
        /** character storage **/
        CharBuffer arena_;
    };
}

/* end LogBuffer.hpp */
