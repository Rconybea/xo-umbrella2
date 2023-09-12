/* @file log_state.hpp */

#pragma once

#include "log_config.hpp"
#include "log_streambuf.hpp"
#include "pad.hpp"
#include <ostream>
#include <memory>   // for std::unique_ptr

namespace xo {
    // track per-thread state associated with nesting logger
    //
    template <typename CharT, typename Traits>
    class state_impl {
    public:
        using log_streambuf_type = log_streambuf<char, std::char_traits<char>>;

    public:
        state_impl();

        std::uint32_t nesting_level() const { return nesting_level_; }

        void incr_nesting() { ++nesting_level_; }
        void decr_nesting() { --nesting_level_; }

        std::ostream & ss() { return ss_; }

        /* call on entry to new scope */
        void preamble(function_style style, std::string_view name1, std::string_view name2);
        /* call before each new log entry */
        void indent(char pad_char);
        /* call on exit from scope */
        void postamble(function_style style, std::string_view name1, std::string_view name2);

        /* write collected output to *p_sbuf */
        void flush2sbuf(std::streambuf * p_sbuf);

        /* discard output, reset write pointer to beginning of buffer */
        void reset_stream() {
            p_sbuf_phase1_->reset_stream();
            p_sbuf_phase2_->reset_stream();
        }

    private:
        /* common implementation for .preamble(), .postamble() */
        void entryexit_aux(function_style style,
                           std::string_view name1,
                           std::string_view name2,
                           char label_char);

    private:
        /* current nesting level for this thread */
        std::uint32_t nesting_level_ = 0;

        /* buffer space for logging
         * (before pretty-printing for scope::log() calls that span multiple lines)
         * reused across tos() and scope::log() calls
         */
        std::unique_ptr<log_streambuf_type> p_sbuf_phase1_;

        /* buffer space for handling scope::log() calls that span multiple lines;
         * inserts extra characters in effort to indent gracefully
         */
        std::unique_ptr<log_streambuf_type> p_sbuf_phase2_;

        /* output stream -- always attached to .p_sbuf_phase1
         * stream inserters for application datatypes will target this stream
         */
        std::ostream ss_;
    }; /*state_impl*/

    constexpr uint32_t c_default_buf_size = 1024;

    template <typename CharT, typename Traits>
    state_impl<CharT, Traits>::state_impl()
        : p_sbuf_phase1_(new log_streambuf_type(c_default_buf_size)),
          p_sbuf_phase2_(new log_streambuf_type(c_default_buf_size)),
          ss_(p_sbuf_phase1_.get())
    {
        assert(p_sbuf_phase1_.get() == ss_.rdbuf());
    } /*ctor*/

    template <typename CharT, typename Traits>
    void
    state_impl<CharT, Traits>::indent(char pad_char)
    {
        //log_streambuf * sbuf = this->p_sbuf_phase1_.get();

#ifdef NOT_IN_USE
        {
            char buf[80];
            ::snprintf(buf, sizeof(buf), "[%02d] ", this->nesting_level_);

            this->ss_ << buf;
            //this->p_sbuf_->sputn(buf, strlen(buf));
        }
#endif

        /* indent to nesting level */
        this->ss_ << pad(this->nesting_level_ * log_config::indent_width, pad_char);
#ifdef OBSOLETE
        for(uint32_t i = 0, n = this->nesting_level_; i<n; ++i) {
            this->ss_ << pad_char;
        }
#endif
    } /*indent*/

    template <typename CharT, typename Traits>
    void
    state_impl<CharT, Traits>::entryexit_aux(function_style style,
                                             std::string_view name1,
                                             std::string_view name2,
                                             char label_char)
    {
        log_streambuf_type * sbuf = this->p_sbuf_phase1_.get();

        sbuf->reset_stream();
        this->indent(' ');

        /* mnemonic for scope entry/exit */
        this->ss_ << label_char;

        if (log_config::indent_width > 1)
            this->ss_ << ' ';

        /* scope name */
        this->ss_ << function_name(style, name1) << name2;
    } /*entryexit_aux*/

    template <typename CharT, typename Traits>
    void
    state_impl<CharT, Traits>::preamble(function_style style,
                                        std::string_view name1,
                                        std::string_view name2)
    {
        this->entryexit_aux(style, name1, name2, '+' /*label_char*/);
    } /*preamble*/

    template <typename CharT, typename Traits>
    void
    state_impl<CharT, Traits>::postamble(function_style style,
                                         std::string_view name1,
                                         std::string_view name2)
    {
        this->entryexit_aux(style, name1, name2, '-' /*label_char*/);
    }  /*postamble*/

    template <typename CharT, typename Traits>
    void
    state_impl<CharT, Traits>::flush2sbuf(std::streambuf * p_sbuf)
    {
        log_streambuf_type * sbuf1 = this->p_sbuf_phase1_.get();
        log_streambuf_type * sbuf2 = this->p_sbuf_phase2_.get();

        /* expecting sbuf to contain one line of output.
         * if it contains multiple newlines,  need to indent
         * after each one.
         *
         * will scan output in *sbuf1,  post-process to *sbuf2,
         * then write *sbuf2 to clog
         */
        char const * s = sbuf1->lo();
        char const * e = s + sbuf1->pos();

        char const * p = s;

        /* point to first space following a non-space character.
         * will indent to just after this space
         */
        char const * space_after_nonspace = nullptr;

        while(true) {
            bool have_nonspace = false;

            /* invariant: s<=p<=e */

            /* for indenting,  looking for first 'space following non-space, on first line', if any */

            while(p < e) {
                if(space_after_nonspace) {
                    ;
                } else {
                    if(*p != ' ')
                        have_nonspace = true;

                    if(have_nonspace && (*p == ' ')) {
                        space_after_nonspace = p;
                    }
                }

                if(*p == '\n') {
                    ++p;
                    break;
                } else {
                    ++p;
                }
            }

            /* p=e or *p=\n */

            /* charseq [s,p) does not contain any newlines,  print it */
            sbuf2->sputn(s, p - s);

            if(p == e) {
                break;
            }

            // {
            //   char buf[80];
            //   snprintf(buf, sizeof(buf), "*** indent=[%d] next=[%c]", this->nesting_level_, *(p+1));
            //
            //   std::clog.rdbuf()->sputn(buf, strlen(buf));
            //}

            /* at least 1 char following newline,  need to indent for it
             * - minimum indent = nesting level;
             * - however if space_after_nonspace defined, indent to that
             */
            uint32_t n_indent = this->nesting_level_;

            if(space_after_nonspace)
                n_indent += (space_after_nonspace - s);

            for(uint32_t i = 0; i < n_indent; ++i)
                sbuf2->sputc(' ');

            s = p;
        }

        /* now write entire contents of *sbuf2 to clog */
        p_sbuf->sputn(sbuf2->lo(), sbuf2->pos());

        /* reset streams for next message */
        this->reset_stream();
    } /*flush2sbuf*/
} /*namespace xo*/

/* end log_state.hpp */
