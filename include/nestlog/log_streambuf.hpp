/* @file log_streambuf.hpp */

#pragma once

#include <iostream>
#include <vector>
#include <cstring>   // e.g. for std::memcpy()
#include <cassert>   // e.g. for std::memcpy()

namespace xo {
    /* recycling buffer for logging.
     * write to self-extending storage array;
     */
    template <typename CharT, typename Traits>
    class log_streambuf : public std::streambuf {
    public:
        log_streambuf(std::uint32_t buf_z) {
            this->buf_v_.resize(buf_z);
            this->reset_stream();
        } /*ctor*/

        std::streamsize capacity() const { return this->buf_v_.size(); }
        char const * lo() const { return this->pbase(); }
        char const * hi() const { return this->lo() + this->capacity(); }
        std::uint32_t pos() const { return this->pptr() - this->pbase(); }

        void reset_stream() {
            char * p_lo = &(this->buf_v_[0]);
            char * p_hi = p_lo + this->capacity();

            /* tells parent our buffer extent */
            this->setp(p_lo, p_hi);
        } /*reset_stream*/

    protected:
        virtual std::streamsize
        xsputn(char const * s, std::streamsize n) override {
                /* s must be an address in [this->lo() .. this->lo() + capacity()] */

                assert(this->hi() >= this->pptr());

#ifdef NOT_USING_DEBUG
                std::cout << "xsputn: pbase=" << (void *)(this->pbase())
                          << ", pptr=" << (void*)(this->pptr())
                          << "(+" << (this->pptr() - this->lo()) << ")"
                          << ", n=" << n << " -> (+" << (this->pptr() + n - this->lo()) << ")"
                          << ", buf_v.size=" << this->buf_v_.size()
                          << std::endl;
#endif
                //std::cout << "xsputn: s=" << quoted(string_view(s, n)) << ", n=" << n << std::endl;

                if (this->pptr() + n > this->hi()) {
                    n = this->hi() - this->pptr();
                    std::memcpy(this->pptr(), s, n);
                } else {
                    std::memcpy(this->pptr(), s, n);
                }
                this->pbump(n);

                return n;
            } /*xsputn*/

        virtual int_type
        overflow(int_type new_ch) override
            {
                char * old_pptr = this->pptr();
                std::streamsize old_n = old_pptr - this->pbase();

                assert(old_n <= static_cast<std::streamsize>(this->buf_v_.size()));

                std::size_t new_z = 2 * this->buf_v_.size();

                this->buf_v_.resize(new_z);
                this->buf_v_[old_n] = new_ch;

                /* 'buffered range' will now be .buf_v[old_n .. new_z] */
                char * p_base = &(this->buf_v_[0]);
                //char * p_lo = &(this->buf_v_[old_n+1]);
                char * p_hi = p_base + this->buf_v_.capacity();

                this->setp(p_base, p_hi);
                this->pbump(old_n + 1);

                return new_ch;
            } /*overflow*/

        /* off.   offset, relative to starting point dir.
         * dir.
         * which. in|out|both
         */
        virtual pos_type seekoff(off_type off,
                                 std::ios_base::seekdir dir,
                                 std::ios_base::openmode which) override {
            //std::cout << "seekoff: off=" << off << ", dir=" << dir << ", which=" << which << std::endl;

            // Only output stream is supported
            if (which != std::ios_base::out)
                throw std::runtime_error("log_streambuf: only output mode supported");

            if (dir == std::ios_base::cur) {
                this->pbump(off);
            } else if (dir == std::ios_base::end) {
                /* .setp(): using for side effect: sets .pptr to .pbase */
                this->setp(this->pbase(), this->epptr());
                this->pbump(off);
            } else if (dir == std::ios_base::beg) {
                /* .setp(): using for side effect: sets .pptr to .pbase */
                this->setp(this->pbase(), this->epptr());
                this->pbump(this->capacity() + off);
            }

            return this->pptr() - this->pbase();
        } /*seekoff*/

    private:
        /* buffered output stored here */
        std::vector<char> buf_v_;
    }; /*log_streambuf*/

} /*namespace xo*/

/* end log_streambuf.hpp */
