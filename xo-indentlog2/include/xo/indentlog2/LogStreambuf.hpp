/** @file LogStreambuf.hpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#include "LogBuffer.hpp"

namespace xo {
    /** @brief Arena-backed streambuf for logging and pretty-printing.
     **/
    class LogStreambuf : public std::streambuf {
    public:
        using ArenaConfig = xo::mm::ArenaConfig;
        using traits_type = std::streambuf::traits_type;
        using Span = xo::mm::span<char>;
        using streamsize = std::streamsize;

    public:
        LogStreambuf(LogBuffer * logbuf);

        /** allocated buffer extned available to hold content (allocated + available) **/
        Span committed_span();

        /** reset stream to empty state, ready to receive output.
         *  Preserves buffer memory allocation.
         **/
        void reset_stream();

        /** expand log buffer size to at least @p new_z chars
         *  Require: new_z <= reserved range of backing arena.
         *  Can fail if physical memory exhausted.
         *  Returns false if expansion failed.
         **/
        bool expand_to(size_t new_z);

        /** verify cross-layer invariant between this streambuf's put-area
         *  pointers {pbase, pptr, epptr} and the backing @ref logbuf_.
         *  Intended for assert() in mutators.  See @ref overflow.
         **/
        bool verify_ok() const;

        /** synchronize streambuf state {pbase, pptr, epptr} in case of
         *  independent writes to @ref logbuf_
         **/
        void _check_update_streambuf_state();

        // inherited from std::streambuf

        /** Write up to @p n characters starting at @p s.
         *  @return number of chars written
         **/
        virtual streamsize xsputn(const char * s, streamsize n) override;
        /** Called when character @p new_ch was not able to be written,
         *  because buffer size exhausted.
         *  If we can expand buffer, will write character.
         *  Otherwise return EOF (i.e. all fallbacks exhausted)
         **/
        virtual int_type overflow(int_type new_ch) override;
        /** Bundles various api modes to get/set current offset within buffer.
         *  - off=0,dir=cur,which=out reads current offset
         *  - off=n,dir=cur,which=out with n!=0 advances relative to current position
         *  - off=n,dir=end,which=out with n<0 advances relative to end of (committed)
         *    buffer space
         **/
        virtual pos_type seekoff(off_type off,
                                 std::ios_base::seekdir dir,
                                 std::ios_base::openmode which) override;
        /** Push (flush) streambuf write position into @ref logbuf_:
         *  inform it of chars written through the sputc fast path
         *  (which bypasses xsputn/overflow) since the last sync.
         *  This is the "push" half of the native<->streambuf handoff;
         *  @ref _check_update_streambuf_state is the "pull" half.
         **/
        virtual int sync() override;

    private:
        /** buffer stsorage **/
        LogBuffer * logbuf_ = nullptr;
    };
} /*namespace xo*/

/* end LogStreambuf.hpp */
