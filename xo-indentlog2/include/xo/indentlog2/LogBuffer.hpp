/** @file LogBuffer.hpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#pragma once

#include "LineState.hpp"
#include "xo/arena/DArena.hpp"

namespace xo {
    using CharBuffer = xo::mm::DArena;
    //using CharBuffer = xo::mm::DArenaVector<char>;

    /** @brief Arena-backed buffer storage for logging and pretty printing **/
    class LogBuffer {
        // Adapted from xo::log_streambuf.
        // 1. Keeping the stateful part
        // 2. Detaching the streambuf inheritance
        // 3. Dropping the CharT parameter, just need utf8
    public:
        using ArenaConfig = xo::mm::ArenaConfig;
        using DArena = xo::mm::DArena;
        using Span = LineState::Span; // = xo::mm::span<char>;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
        using size_t = std::size_t;

    public:
        LogBuffer(const ArenaConfig & config, bool debug_flag);

        bool debug_flag() const { return debug_flag_; }

        /** allocated buffer extent available to hold content (allocated + available) **/
        Span committed_span();

        /** visit mapped storage pools; include LogBuffer because it's arena-backed **/
        void visit_pools(const MemorySizeVisitor & fn) const;

        /** expand buffered character storage to at least @p new_z.
         *  @return true on success, false if oom
         **/
        bool expand_to(size_t new_z);

        /** reset to nominal state, ready to receive output,
         *  but with no buffered content. Also resets @ref lstate_
         **/
        void reset_buffer();

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
