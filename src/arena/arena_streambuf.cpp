/** @file arena_streambuf.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "arena_streambuf.hpp"

namespace xo {
    namespace mm {

        std::uint32_t
        arena_streambuf::lpos() const
        {
            if (debug_flag_) {
                std::cerr << "log_streambuf::lpos: enter" << std::endl;
            }

            // logically-const. lazy implementation
            arena_streambuf * self = const_cast<arena_streambuf *>(this);

            self->_check_update_local_state();

            return pos() - solpos_ - color_escape_chars_;
        }

        auto
        arena_streambuf::checkpoint() const -> rewind_state
        {
            // logically-const. lazy implementation
            arena_streambuf * self = const_cast<arena_streambuf *>(this);

            self->_check_update_local_state();

            return rewind_state(solpos_, color_escape_chars_, pos());
        }

        void
        arena_streambuf::reset_stream()
        {
            assert(arena_);
            assert(arena_->committed() > 0);

            char * p_lo = (char *)(arena_->lo_);
            char * p_hi = (char *)(arena_->limit_);

            /* tells parent our buffer extent */
            this->setp(p_lo, p_hi);

            this->local_ppos_ = 0;
            this->solpos_ = 0;
            this->color_escape_chars_ = 0;
            this->color_escape_start_ = nullptr;
        }
    
        void
        arena_streambuf::rewind_to(rewind_state s)
        {
            if (debug_flag_) {
                std::cout << "rewind_to: pos " << pos() << "->" << s.pos
                << " solpos " << solpos_ << "->" << s.solpos
                << " color_esc " << color_escape_chars_ << "->" << s.color_escape_chars
                << std::endl;
            }

            /* .setp(): using just for side effect: sets .pptr to .pbase */
            this->setp(this->pbase(), this->epptr());
            /* advance pptr to saved position */
            this->pbump(s.pos);

            this->local_ppos_ = this->pptr() - this->pbase();
            this->solpos_ = s.solpos;
            this->color_escape_chars_ = s.color_escape_chars;
            /* assuming we never try to capture rewind state with incomplete color escape */
            this->color_escape_start_ = nullptr;
        }

        void
        arena_streambuf::expand_to(std::size_t new_z)
        {
            char * old_pptr = pptr();
            std::streamsize old_n = old_pptr - pbase();

            assert(old_n <= static_cast<std::streamsize>(arena_->allocated()));
            assert(new_z > arena_->committed());

            /* note: local_ppos_ invariant across expand_to() */

            arena_->expand(new_z);

            char * p_base = (char *)(arena_->lo_);
            char * p_hi = (char *)(arena_->limit_);

            this->setp(p_base, p_hi);
            this->pbump(old_n);
        }

        std::streamsize
        arena_streambuf::xsputn(const char * s, std::streamsize n)
        {
            /* s must be an address in [this->lo() .. this->lo() + capacity()] */

            assert(hi() >= pptr());

            if (pptr() + n > hi()) {
                std::size_t new_z = std::max(2 * arena_->committed(), std::size_t(this->pos() + n + 1));

                if (new_z > arena_->reserved())
                    new_z = arena_->reserved();

                this->expand_to(new_z);
            }

            if (debug_flag_) {
                std::cout << "xsputn: pbase=" << (void *)(this->pbase())
                << ", pptr=" << (void*)(this->pptr())
                << "(+" << (this->pptr() - this->lo()) << ")"
                << ", n=" << n << " -> (+" << (this->pptr() + n - this->lo()) << ")"
                << ", arena.size=" << this->arena_->committed()
                << std::endl;
            }

            std::streamsize ncopied = 0;

            if (this->pptr() + n > this->hi()) {
                ncopied = this->hi() - this->pptr();
            } else {
                ncopied = n;
            }

            if (false /*debug_flag_*/) {
                std::cout << "xsputn: copying ncopied=" << ncopied << " (/n=" << n << ") bytes into range [lo,hi)"
                << ", lo=" << (void*)this->pptr()
                << ", hi=" << (void*)(this->pptr() + n)
                << std::endl;
            }

            std::memcpy(this->pptr(), s, ncopied);

            this->pbump(ncopied);

            /* now {pbase, pptr} consistent with new input */

            this->_check_update_local_state();

            return ncopied;
        }

        auto
        arena_streambuf::overflow(int_type new_ch) -> int_type
        {
            char * old_base = this->pbase();
            char * old_pptr = this->pptr();
            /* #of chars buffered */
            std::streamsize old_n = old_pptr - old_base;

            assert(old_n <= static_cast<std::streamsize>(arena_->committed()));

            //        if (debug_flag_) {
            //            std::cout << "overflow: new_ch=" << quoted_char(new_ch) << std::endl;
            //        }

            /* increase buffer size */
            this->expand_to(2 * arena_->committed());

            arena_->lo_[old_n] = static_cast<std::byte>(new_ch);
            this->pbump(1);

            if ((new_ch == static_cast<int_type>('\n')) || (new_ch == static_cast<int_type>('\r'))) {
                this->solpos_ = this->pos();

                // what if new_ch starts color escape ?
            }

            if (new_ch == std::char_traits<char>::eof()) {
                /* reminder: returning eof sets badbit on ostream */
                return std::char_traits<char>::not_eof(new_ch);
            } else {
                return new_ch;
            }
        }

        auto
        arena_streambuf::seekoff(off_type off,
                                 std::ios_base::seekdir dir,
                                 std::ios_base::openmode which) -> pos_type
        {
            //std::cout << "seekoff: off=" << off << ", dir=" << dir << ", which=" << which << std::endl;
            if (debug_flag_) {
                std::cout << "seekoff(off,dir,which)" << std::endl;
            }

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
    
    } /*namespace mm*/
} /*namespace xo*/

/* end arena_streambuf.cpp */
