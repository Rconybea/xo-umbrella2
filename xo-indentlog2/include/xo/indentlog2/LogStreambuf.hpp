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
        using traits_type = std::streambuf::traits_type;
        using Span = xo::mm::span<char>;
        using streamsize = std::streamsize;

    public:
        LogStreambuf(const ArenaConfig & config, bool debug_flag);

        /** allocated buffer extned available to hold content (allocated + available) **/
        Span committed_span();

        /** reset stream to empty state, ready to receive output.
         **/
        void reset_stream();

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

    private:
        LogBuffer logbuf_;
    };
} /*namespace xo*/

/* end LogStreambuf.hpp */
