/** @file LogStreambuf.cpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#include "LogStreambuf.hpp"

namespace xo {
    using std::streamsize;
    using std::cerr;
    using std::endl;

    void
    LogStreambuf::reset_stream()
    {
        /** reset line accounting and buffer pointers **/
        logbuf_.reset_buffer();

        auto buf_span = logbuf_.committed_span();

        /* tells parent streambuf our current buffer extent */
        this->setp(buf_span.lo(), buf_span.hi());
    }

    streamsize
    LogStreambuf::xsputn(const char * s, streamsize n)
    {
        /* s must be an address in [this->lo() .. this->lo() + capacity()] */

        LogBuffer::Span c = logbuf_.committed_span();

        assert(c.hi() >= pptr());

        if (pptr() + n > c.hi()) {
            auto new_z = c.size() + n;

            logbuf_.expand_to(new_z);

            c = logbuf_.committed_span();
        }

        if (logbuf_.debug_flag()) {
            std::cerr << "LogStreambuf::xsputn: pbase=" << (void *)(this->pbase())
                      << ", pptr=" << (void*)(this->pptr())
                      << "(+" << (this->pptr() - c.lo()) << ")"
                      << ", n=" << n << " -> (+" << (this->pptr() + n - c.lo()) << ")"
                      << ", buf_v.size=" << c.size()
                      << std::endl;
        }

        // #of bytes to copy
        std::streamsize n2copy = 0;

        if (this->pptr() + n > c.hi()) {
            n2copy = c.hi() - this->pptr();
        } else {
            n2copy = n;
        }

        if (false /*debug_flag_*/) {
            cerr << "LogStreambuf::xsputn:"
                 << " copying n2copy=" << n2copy << " (/n=" << n << ") bytes into range [lo,hi)"
                 << ", lo=" << (void*)this->pptr()
                 << ", hi=" << (void*)c.hi()
                 << endl;
        }

        std::memcpy(this->pptr(), s, n2copy);

        this->pbump(n2copy); // advances this->pptr()

        logbuf_._check_update_local_state(this->pptr());

        return n2copy;
    }

    auto
    LogStreambuf::overflow(int_type new_ch) -> int_type
    {
        char * old_base = this->pbase();
        char * old_pptr = this->pptr();
        /* #of chars buffered */
        std::streamsize old_n = old_pptr - old_base;

        auto z0 = logbuf_.committed_span().size();

        assert(old_n <= static_cast<std::streamsize>(logbuf_.committed_span().size()));

        if (logbuf_.debug_flag()) {
            cerr << "LogStreambuf::overflow: new_ch=" << quoted_char(new_ch) << endl;
        }

        /* increase buffer size.  Arena will add at least one page */
        if (logbuf_.expand_to(z0 + 1)) [[likely]] {
            *(this->pptr()) = new_ch;

            this->pbump(1); // advances this->pptr();
            logbuf_._check_update_local_state(this->pptr());
        } else {
            /* here we return eof to indicate something out-of-space */
            return traits_type::eof();
        }

        if (new_ch == traits_type::eof()) [[unlikely]] {
            /* reminder: If streambuf is inside an ostream, then
             *           returning eof here would set badbit on that ostream.
             *           In this case eof is deliberate,
             *           but need explicit reassurance.
             */
            return traits_type::not_eof(new_ch);
        } else {
            return new_ch;
        }
    }

    auto
    LogStreambuf::seekoff(off_type off,
                          std::ios_base::seekdir dir,
                          std::ios_base::openmode which) -> pos_type
    {
        // LogStreambuf is output-only
        if (which != std::ios_base::out)
            throw std::runtime_error("LogStreambuf: only output mode supported");

        char * pbase = this->pbase();

        if (dir == std::ios_base::cur) {
            this->pbump(off);
        } else if (dir == std::ios_base::beg) {
            assert(off >= 0);

            /* using .setp for side-effect: sets .pptr to .pbase */
            this->setp(pbase, this->epptr());
            this->pbump(off);
        } else if (dir == std::ios_base::end) {
            assert(off <= 0);

            /* using .setp for side-effect: sets .pptr to .pbase **/
            this->setp(pbase, this->epptr());
            this->pbump(this->committed_span().size() + off);
        }

        return this->pptr() - pbase;
    }
}

/* end LogStreambuf.cpp */
