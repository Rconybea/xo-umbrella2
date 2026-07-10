/** @file LogBuffer.hpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#pragma once

#include "LineState.hpp"
#include "xo/arena/DArena.hpp"

namespace xo {
    using CharBuffer = xo::mm::DArena;

    /** @brief Arena-backed buffer storage for logging and pretty printing **/
    class LogBuffer {
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
        /** Create instance using @p config for @ref buf_v_ **/
        LogBuffer(const ArenaConfig & config, bool debug_flag);

        bool debug_flag() const { return debug_flag_; }
        size_t lpos() const { return lstate_.lpos(); }

        /** allocated buffer extent available to hold content (allocated + available) **/
        Span committed_span();
        /** used buffer extent **/
        Span used_span();
        /** available (contiguous) buffer extent **/
        Span available_span();

        /** visit mapped storage pools; include LogBuffer because it's arena-backed **/
        void visit_pools(const MemorySizeVisitor & fn) const;

        /** expand buffered character storage to at least @p new_z.
         *  @return true on success, false if oom
         **/
        bool expand_to(size_t new_z);

        /** write newline and @p indent spaces **/
        void newline_indent(uint32_t indent);
        /** write span @p x to buffer **/
        void write_span(ConstSpan x);

        /** reset to nominal state, ready to receive output,
         *  but with no buffered content. Also resets @ref lstate_
         **/
        void reset_buffer();

        /** require room for @p x chars **/
        bool _require_avail(uint32_t x);

        /** synchronize line accountant @ref lstate_ when dirty
         *  (because chars possibly added to @ref buf_v_)
         **/
        void _check_update_local_state(const char * pptr);

    private:
        /** @defgroup LogBuffer-instance-vars **/
        ///@{

        /** character storage **/
        CharBuffer buf_v_;
        /** checkpoint for realloc **/
        DArena::Checkpoint buf_ckp_;
        /** start of usable buffered memory **/
        char * pbase_ = nullptr;
        /** current write pointer **/
        char * pptr_ = nullptr;
        /** end of usable buffered memory (can be expanded) **/
        char * epptr_ = nullptr;
        /** line state computed incrementally from contents of @ref buf_v_ **/
        LineState lstate_;
        /** true to debug LogBuffer itself **/
        bool debug_flag_ = false;

        ///@}
    };
}

/* end LogBuffer.hpp */
