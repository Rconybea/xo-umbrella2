/** @file arena_streambuf.hpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include "DArena.hpp"
//#include "print/quoted_char.hpp"
#include <iostream>
#include <string_view>
#include <vector>
#include <cstring>   // e.g. for std::memcpy()
#include <cstdint>
#include <cassert>

namespace xo {
    namespace mm {
        /** @brief Arena-based buffer for logging and pretty-printing
         *
         *  Arena-based using mmap
         *  Write to self-extending storage array
         *  Track position relative to start of line
         **/
        class arena_streambuf : public std::streambuf {
        public:
            struct rewind_state {
                explicit rewind_state(std::size_t solpos, std::size_t color_esc, std::uint32_t p)
                : solpos{solpos}, color_escape_chars{color_esc}, pos{p} {}

                std::size_t solpos = 0;
                std::size_t color_escape_chars = 0;
                std::uint32_t pos = 0;
            };

        public:
            /** arena should be ready-to-allocate i.e. have committed > 0 **/
            arena_streambuf(DArena * arena, bool debug_flag = false) : arena_{arena}, debug_flag_{debug_flag} {
                this->reset_stream();
            } /*ctor*/

            std::streamsize capacity() const { return arena_->committed(); }
            const char * lo() const { return this->pbase(); }
            const char * hi() const { return this->lo() + this->capacity(); }
            std::uint32_t pos() const { return this->pptr() - this->pbase(); }

            /** output position (relative to pbase) when local state last computed. Exposed here for unit tests **/
            std::size_t _local_ppos() const { return local_ppos_; }
            /** position (relative to pbase) one character after last \n or \r. For unit tests **/
            std::uint32_t _solpos() const { return solpos_; }
            /** start of incomplete color-escape sequence **/
            const char * _color_escape_start() const { return color_escape_start_; }
            /** number of non-printing chars after @ref solpos_ from completed color-escape sequences **/
            std::uint32_t _color_escape_chars() const { return color_escape_chars_; }

            /** number of visible characters since start of line (last \n or \r) **/
            std::uint32_t lpos() const;

            rewind_state checkpoint() const;

            bool debug_flag() const { return debug_flag_; }

            operator std::string_view () const { return std::string_view(this->pbase(), this->pptr()); }

            void reset_stream();

            void rewind_to(rewind_state s);

        protected:
            /** expand buffer storage (by 2x), preserve current contents **/
            void expand_to(std::size_t new_z);

            virtual std::streamsize xsputn(const char * s, std::streamsize n) override;

            virtual int_type overflow(int_type new_ch) override;

            /* off.   offset, relative to starting point dir.
             * dir.
             * which. in|out|both
             *
             * Note that off=0,dir=cur,which=out reads offset
             */
            virtual pos_type seekoff(off_type off,
                                     std::ios_base::seekdir dir,
                                     std::ios_base::openmode which) override;

        private:
            void _update_local_state_char(const char * p_lo, const char * p)
            {
                if ((*p == '\n') || (*p == '\r')) {
                    this->solpos_ = (p+1 - this->pbase());
                    /* reset, since these chars relevant as correction to solpos */
                    this->color_escape_chars_ = 0;
                    /* -> incomplete color escape, broken by newline */
                    this->color_escape_start_ = nullptr;
                } else if (*p == '\033') {
                    if (debug_flag_) [[unlikely]] {
                        std::cout << "xsputn: \\033 at p-p_lo=" << (p - p_lo) << std::endl;
                    }
                    this->color_escape_start_ = p;
                } else if (this->color_escape_start_ != nullptr) {
                    if (*p == 'm') {
                        /* escape seq non-printing including both endpoints */
                        std::int64_t esc_chars = (p+1 - color_escape_start_);

                        this->color_escape_chars_ += esc_chars;

                        if (debug_flag_) [[unlikely]] {
                            std::cout << "xsputn: m at p-p_lo" << (p - p_lo) << " +" << esc_chars
                            << " -> color_escape_chars=" << color_escape_chars_ << std::endl;
                        }
                        this->color_escape_start_ = nullptr;
                    } else if (!isdigit(*p) && (*p != '[') && (*p != ';')) {
                        /* not color escape after all */
                        this->color_escape_start_ = nullptr;
                    }
                }
            }

            /** recognize stale local state vars:
             *  @ref solpos_, @ref color_escape_chars_, @ref color_escape_start_.
             *
             *  Require:
             *  - {pbase, pptr} in consistent state
             *  Promise:
             *  - @c local_ppos_ + @c pbase = @c pptr
             *  - @c solpos_, @c color_escape_chars_, @c color_escape_start_ all up-to-date
             **/
            void _check_update_local_state() {
                const char * p0 = this->pbase();
                const char * pn = this->pptr();

                if (debug_flag_) {
                    std::cerr << "_check_update_local_state:" << std::endl;
                    std::cerr << "  buf: (p0=" << (void*)p0 << ", pn=" << (void*)pn << ")" << std::endl;
                    std::cerr << "  solpos_=" << solpos_ << ", color_escape_chars_=" << color_escape_chars_ << std::endl;
                }

                if (p0 + local_ppos_ == pn) [[likely]] {
                    // solpos_, color_escape_chars_, color_escape_start_ all up-to-date
                } else {
                    // [pnew, pn): input that hasn't been incorporated into
                    //   {solpos_, color_escape_chars_, color_escape_start_)

                    const char * pnew = this->pbase() + this->local_ppos_;

                    if (debug_flag_) {
                        std::cerr << "_check_update_local_state: range: (pnew=" << (void*)pnew << ", pn=" << (void*)pn << ")" << std::endl;
                    }

                    for(const char * p = pnew; p < pn; ++p) {
                        this->_update_local_state_char(p0, p);
                    }
                }

                // solpos_, color_escape_chars_, color_escape_start_ all up-to-date
                // for current buffered contents

                this->local_ppos_ = pn - p0;

                if (debug_flag_) {
                    std::cerr << "_check_update_local_state: pos=" << pos();
                    std::cerr << ", solpos=" << solpos_;
                    std::cerr << ", color_escape_chars=" << color_escape_chars_ << std::endl;
                }

                assert(pos() >= solpos_ + color_escape_chars_);
            }

        private:
            /*
             *   pbase: start of buffered text.  Thils will be arena_->lo_
             *
             *
             *   pbase                                          pptr             epptr
             *   v                  >e1<  >e2<                     v                 v
             *   |xx\xxEEExxx\xxxxxxxEExxxxEExxxxxxxEExxx\xEExxxxxx..................|
             *                ^                 ^<------new------->
             *           solpos        local_ppos
             *
             *   solpos           : first character after newline (stale)
             *   color_escape_pos : e1+e2+.. (stale)
             *   new              : new characters not reflected
             *                      in local_ppos_, color_escape_chars_ etc.
             *
             *   Legend:
             *    [\] newline
             *    [x] visible character
             *    [E] color escape chars
             *
             *
             *   after _check_update_local_state():
             *
             *
             *   pbase                                          pptr             epptr
             *   v                                        >e1<     v                 v
             *   |xx\xxEEExxx\xxxxxxxEExxxxEExxxxxxxEExxx\xEExxxxxx..................|
             *                                            ^        ^
             *                                       solpos        local_ppos
             *
             */

            /** @defgroup logstreambuf-instance-vars **/
            ///@{

            /** value of pptr (relative to pbase) when _check_update_local_state() last ran **/
            std::size_t local_ppos_ = 0;
            /** position (relative to pbase) one character after last \n or \r.
             *  Use to drive @ref lpos.  This _has_ to be lazy, since
             *  xsputn() isn'g guaranteed to be called when there's room in
             *  in buffer.
             **/
            std::size_t solpos_ = 0;
            /** number of non-printing chars after @ref solpos_, from
             *  completed color escape sequences.
             *  (ansi color escapes = text between '\033' and 'm')
             **/
            std::size_t color_escape_chars_ = 0;
            /** non-null: start of incomplete color escape sequence **/
            const char * color_escape_start_ = nullptr;

            /** buffered output stored here.
             *  We don't use arena's allocation api, just treat as a block of available memory
             **/
            DArena * arena_ = nullptr;;
            /** true to debug log_streambuf itself **/
            bool debug_flag_ = false;

            ///@}
        }; /*log_streambuf*/

    } /*namespace mm*/
} /*namespace xo*/

/* end arena_streambuf.hpp */

