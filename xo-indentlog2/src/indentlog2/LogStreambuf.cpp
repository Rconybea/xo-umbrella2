/** @file LogStreambuf.cpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#include "LogStreambuf.hpp"

namespace xo {
    using std::streamsize;
    using std::cerr;
    using std::endl;

    LogStreambuf::LogStreambuf(LogBuffer * logbuf)
      : logbuf_{logbuf}
    {
        if (logbuf_)
            this->_check_update_streambuf_state();
    }

    auto
    LogStreambuf::committed_span() -> Span
    {
        if (logbuf_) [[likely]] {
            return logbuf_->committed_span();
        } else {
            return Span();
        }
    }

    void
    LogStreambuf::reset_stream()
    {
        /** reset line accounting and buffer pointers **/
        logbuf_->reset_buffer();

        auto buf_span = logbuf_->committed_span();

        /* tells parent streambuf our current buffer extent */
        this->setp(buf_span.lo(), buf_span.hi());
    }

    streamsize
    LogStreambuf::xsputn(const char * s, streamsize n)
    {
        // note: in case independent writes to LogBuffer state
        this->_check_update_streambuf_state();

        // note: correct also in edge case where {epptr, pptr} are nullptr
        if (this->epptr() - this->pptr() < n) {
            this->expand_to((this->pptr() - this->pbase()) + n);
        }

        // #of bytes to copy
        std::streamsize n2copy = std::min<streamsize>(n, this->epptr() - this->pptr());

        if (n2copy > 0) [[likely]] {
            std::memcpy(this->pptr(), s, n2copy);
            this->pbump(n2copy); // advances this->pptr()

            logbuf_->_check_update_local_state(this->pptr());
        }

        return n2copy;
    }

    auto
    LogStreambuf::overflow(int_type new_ch) -> int_type
    {
        // note: in case independent writes to LogBufferState
        this->_check_update_streambuf_state();

        assert(this->verify_ok());

        auto z0 = logbuf_->committed_span().size();

        if (logbuf_->debug_flag()) {
            cerr << "LogStreambuf::overflow: new_ch=" << quoted_char(new_ch) << endl;
        }

        /* increase buffer size.  Arena will add at least one page */
        if (this->expand_to(z0 + 1)) [[likely]] {
            *(this->pptr()) = new_ch;

            this->pbump(1); // advances this->pptr();
            logbuf_->_check_update_local_state(this->pptr());
        } else {
            /* here we return eof to indicate something out-of-space */
            return traits_type::eof();
        }

        assert(this->verify_ok());

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

            if (pbase) [[likely]] {
                /* using .setp for side-effect: sets .pptr to .pbase **/
                this->setp(pbase, this->epptr());
                this->pbump(this->committed_span().size() + off);
            } else {
                /* buffer not allocated yet */
            }
        }

        return this->pptr() - pbase;
    }

    bool
    LogStreambuf::expand_to(size_t new_z)
    {
        if (!logbuf_->expand_to(new_z))
            return false;

        // current write offset; 0 when uninitialized
        off_type off = this->pptr() - this->pbase();

        auto c = logbuf_->committed_span();

        // note: setp clobbers pptr -> need pbump to restore it
        this->setp(c.lo(), c.hi());
        this->pbump(off);

        return true;
    }

    void
    LogStreambuf::_check_update_streambuf_state()
    {
        auto used = logbuf_->used_span();
        auto avail = logbuf_->available_span();

        auto pbase = this->pbase(); (void)pbase;
        auto pptr = this->pptr();
        auto epptr = this->epptr();

        // pbase immutable once established
        assert(!pbase || (pbase == used.lo()));

        if (used.lo()) [[likely]] {
            if ((pptr < used.hi()) || (epptr < avail.hi())) {
                // catch up with separately-modified logbuf_ state
                this->setp(used.lo(), avail.hi());
                this->pbump(used.size());
            }
        }
    }

    int
    LogStreambuf::sync()
    {
        /* push: inform LogBuffer of chars written through the sputc fast path
         * (which bypasses xsputn/overflow) since the last sync.
         */
        if (logbuf_ && (this->pptr() > logbuf_->used_span().hi())) {
            logbuf_->_check_update_local_state(this->pptr());
        }

        return 0;
    }

    bool
    LogStreambuf::verify_ok() const
    {
        const char * B = this->pbase();
        const char * P = this->pptr();
        const char * E = this->epptr();

        auto c = logbuf_->committed_span();

        // 1. put area well-formed.
        //    Also holds in the lazy state, where B == P == E == nullptr.
        if (!((B <= P) && (P <= E)))
            return false;

        // 2. both layers agree on the committed extent
        if ((B != c.lo()) || (E != c.hi()))
            return false;

        // 3. LogBuffer write pointer mirrors the streambuf write pointer
        if (logbuf_->used_span().hi() != P)
            return false;

        // 4. line accountant has consumed exactly up to the write pointer
        if (c.lo() + logbuf_->local_ppos() != P)
            return false;

        return true;
    }
}

/* end LogStreambuf.cpp */
