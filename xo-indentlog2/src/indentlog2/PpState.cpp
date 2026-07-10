/** @file PpState.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpState.hpp"
#include "print/PpToken.hpp"
#include <cassert>
#include <cstring>

namespace xo {
    using xo::mm::ArenaConfig;
    using std::max;
    using std::min;

    namespace print {
        PpState::PpState(const PpConfig & cfg)
            : config_{cfg},
              tk_buffer_{DArena::map(ArenaConfig()
                                     .with_name("pp-tk-buffer")
                                     .with_size((sizeof(PpStringToken) + 18)
                                                * cfg.hard_right_margin()))},
              scan_stack_{ScanStack::map(ArenaConfig()
                                         .with_name("pp-scan-stack")
                                         .with_size(cfg.hard_max_nesting()))},
              print_stack_{PrintStack::map(ArenaConfig()
                                           .with_name("pp-print-stack")
                                           .with_size(cfg.hard_max_nesting()))}
        {
            // We reserve virtual memmory for worst-case tk_buffer size.
            // sizeof(PpStringToken) should be 12 bytes;
            // so a PpStringToken with one visible character will use 16 bytes
            // including 3 padding bytes.
            // Worst-case would be 13 bytes for a non-visible rgb ansi color escape,
            // + 4 bytes for end color -> 12 + 13 + 1 + 4 = 30 bytes.
            //                                 <---18--->
            //
            // Per-column this gives us max token buffer memory needed
            // with hard max 2048 columns: 30x2048 ~ 60KB.
            //
            // With 135 columns max would be 30x135 < 4k; in practice this
            // is a very conservative upper bound on actual token buffer needed.
            //
            // So likely reserve ~ 64kb memory, but in practice use one page
            //
            // For {scan stack, print stack}, max nesting depth is unlimited,
            // rely on configured max.
        }

        void
        PpState::connect_output(LogBuffer * p_out)
        {
            p_out_ = p_out;
        }

        void
        PpState::put_cstr(const char * c_str)
        {
            // plan:
            // 1. append PpStringToken for c_str to tk_buffer_
            // 2. update {scan_viz_total, scan_total}
            // 3. print from print_stack_

            uint32_t c_str_z = ::strlen(c_str);
            uint32_t tk_z = PpStringToken::alloc_size(c_str_z);

            void * mem = this->alloc(tk_z);

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            uint32_t viz_z = count_visible_chars(c_str, c_str + c_str_z);
            uint32_t mem_z = tk_z - sizeof(PpStringToken);

            PpStringToken * tk
                = new (mem) PpStringToken(viz_z, c_str_z, mem_z, c_str);

            scan_viz_total_ += tk->tk_viz_len();
            scan_total_ += tk->tk_len();

            // (note: noop unless scan stack is empty)
            this->check_print_ready();
        }

        void
        PpState::begin()
        {
            void * mem = this->alloc(sizeof(PpToken));

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            PpToken * tk = new (mem) PpToken(k_begin, 0, 0);

            // 1. scan-totals unchanged, since no size info
            // 2. begin-token cannot trigger printing,
            //    since doesn't carry any size information.

            (void)tk;
        }

        void
        PpState::split()
        {
            void * mem = this->alloc(sizeof(PpToken));

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            PpToken * tk = new (mem) PpToken(k_break, 0, 0);

            // 1. scan-totals unchanged, since no size info
            // 2. begin-token cannot trigger printing,
            //    since doesn't carry any size information.

            (void)tk;
        }

        void
        PpState::end()
        {
            void * mem = this->alloc(sizeof(PpToken));

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            PpToken * tk = new (mem) PpToken(k_end, 0, 0);

            (void)tk;

            // 1. end-token matches a begin-token
            //    (specifically the token indexed by top of scan stack).
            // 2. Also determines size for that begin-token
            // 3. Possibly unnlocks printing

            if (scan_stack_.size() == 0) [[unlikely]] {
                assert(false);
                return;
            }

            PpToken * begin_token = (PpToken *)((char *)tk_buffer_.lo_ + scan_stack_.back());

            // reminder: just pops the index, doeesn't invalidate begin_token
            scan_stack_.pop_back();

            if ((begin_token->tk_flags() & k_type_mask) != k_begin) [[unlikely]] {
                assert(false);
                return;
            }

            // Size for begin_token is sum of strings scanned until corresponding end token.

            if ((scan_viz_total_ < print_viz_total_)
                || (scan_total_ < print_total_)) [[unlikely]] {

                assert(false);
                return;
            }

            uint32_t tk_viz_z = scan_viz_total_ - print_viz_total_;
            uint32_t tk_z = scan_total_ - print_total_;

            begin_token->establish_size(tk_viz_z, tk_z);

            this->check_print_ready();
        }

        void
        PpState::check_print_ready()
        {
            // plan:
            // 1. loop over tokens, starting from print_ix,
            //    as long as token length is defined
            //    stop when we encounter token with unknown length.
            // 2. for each token:
            //    2.1. if it's a string or end, print immediately
            //    2.2. if it's a break, choose based on
            //         top of print_stack
            //    2.3. if it's a begin, irrevocably set
            //         break flag based on whether it fits

            while (print_ix_ != scan_ix_) {
                PpToken * token = (PpToken *)((char *)tk_buffer_.lo_ + print_ix_);

                if (!token->has_unknown_size()) {
                    // need to know next token size before we
                    // print it.
                    break;
                }

                switch (token->tk_flags() & k_type_mask)
                {
                case k_string:
                    {
                        PpStringToken * str = (PpStringToken *)token;

                        p_out_->write_span(str->span());

                        print_viz_total_ += str->tk_viz_len();
                        print_total_ += str->tk_len();
                    }
                    break;
                case k_begin:
                    {
                        auto lpos = p_out_->lpos();

                        bool f = ((token->tk_viz_len() > 0)
                                  && ((lpos + token->tk_viz_len()
                                       < config_.soft_right_margin())));

                        token->set_fits_flag(f);
                        print_stack_.push_back(print_ix_);
                    }
                    break;
                case k_break:
                    {
                        uint32_t parent_ix = print_stack_.back();
                        PpToken * parent = (PpToken *)((char *)tk_buffer_.lo_ + parent_ix);

                        if (parent->tk_flags() & k_fits) {
                            // if parent fits, then break is a no-op
                        } else {
                            uint32_t indent_z = (print_stack_.size()
                                                 * config_.indent_width());

                            // if parent doesn't fit, then break is newline+indent
                            p_out_->newline_indent(indent_z);
                            print_viz_total_ += (1 + indent_z);
                            print_total_ += (1 + indent_z);
                        }
                    }
                    break;
                case k_end:
                    print_stack_.pop_back();
                    break;
                }

                print_ix_ += token->alloc_size();

                if (print_ix_ == extent_) {
                    extent_ = 0;    // discard buffer wrap, no longer needed
                    print_ix_ = 0;
                }
            }
        }

        uint32_t
        PpState::count_visible_chars(const char * lo, const char * hi) const
        {
            // number of visible chars
            uint32_t z = 0;
            // >0 within color esacpe */
            uint32_t color_z = 0;

            for (const char * p = lo; p < hi; ++p) {
                if (color_z > 0) {
                    if (*p == 'm') {
                        // end color escape
                        color_z = 0;
                    } else if (isdigit(*p) || (*p != '[') || (*p != ';')) {
                        // allowed chars within color escape
                        ++color_z;
                    } else {
                        // char not allowed in color escape,
                        // abandon that interpretation
                        z += color_z;
                        color_z = 0;
                    }
                } else if (*p == '\033') {
                    ++color_z;
                } else {
                    ++z;
                }
            }

            return z;
        }

        void *
        PpState::alloc(uint32_t z)
        {
            if (print_ix_ <= scan_ix_) {
                // buffer currently in one segment [print_ix_, scan_ix_)

                uint32_t avail1_z = tk_buffer_.committed() - scan_ix_;

                if (z <= avail1_z) {
                    void * retval = (char *)tk_buffer_.lo_ + scan_ix_;
                    scan_ix_ += z;
                    return retval;
                } else if (z < print_ix_) {
                    // wrap: start 2nd segment in [tk_buffer_.lo_, print_ix_)
                    extent_ = scan_ix_;
                    scan_ix_ = 0;

                    return tk_buffer_.lo_;
                }

                // fall through
            } else if (scan_ix_ < print_ix_) {
                // buffer in wrapped state (two segments)

                uint32_t avail2_z = print_ix_ - scan_ix_ - 1;

                if (z <= avail2_z) {
                    void * retval = tk_buffer_.lo_ + scan_ix_;
                    scan_ix_ += z;
                    return retval;
                }

                // fall through
            }

            return this->expand_for(z);
        }

        uint32_t
        PpState::available() const
        {
            // Logically, print_ix_ is always chasing scan_ix_

            if (print_ix_ <= scan_ix_) {
                // buffer in one segment
                //  [print_ix_, scan_ix_)

                return tk_buffer_.committed() - scan_ix_;
            } else if (scan_ix_ < print_ix_) {
                // buffer split into two segments:
                //   [print_ix_, extent_) and [0, scan_ix_)

                // -1 so we don't confuse {empty, full} buffers
                return print_ix_ - scan_ix_ - 1;
            } else {
                return 0;
            }
        }

        void *
        PpState::expand_for(uint32_t z)
        {
            z = std::max(z, scan_ix_);

            // expand buffer to make room.
            // If it's wrapped, we're going to insist on unwrapping it.

            if (!tk_buffer_.expand(tk_buffer_.committed() + z, "PpState::expand")) {
                assert(false);
                return nullptr;
            }

            if (print_ix_ <= scan_ix_) {
                if (scan_ix_ + z <= tk_buffer_.committed()) [[likely]]
                    return tk_buffer_.lo_ + scan_ix_;

                // fall through
            } else {
                // 1. buffer split into two segments
                //      [print_ix, extent) and [0, scan_ix_).
                // 2. new memory appears added between the two segments
                //    use to paste them back together

                uint32_t avail_z = tk_buffer_.committed() - extent_;
                uint32_t copy_z = min(scan_ix_, avail_z);

                ::memcpy(tk_buffer_.lo_ + extent_,
                         tk_buffer_.lo_,
                         copy_z);

                this->reindex_stacks(((const char *)tk_buffer_.lo_) + extent_,
                                     (const char *)tk_buffer_.lo_,
                                     copy_z);

                scan_ix_ = extent_ + copy_z;
                extent_ = scan_ix_;

                if (scan_ix_ + z < tk_buffer_.committed()) {
                    return tk_buffer_.lo_ + scan_ix_;
                } else if (z < print_ix_) {
                    // wrap: start 2nd segment in [0, print_ix_)
                    extent_ = scan_ix_;
                    scan_ix_ = 0;

                    return tk_buffer_.lo_;
                }
            }

            assert(false);
            return nullptr;
        }

        void
        PpState::reindex_stacks(const char * tk_dest,
                                      const char * tk_src,
                                      uint32_t z)
        {
            int32_t offset = tk_dest - tk_src;

            for (auto & ix : scan_stack_) {
                const char * p = ((const char *)tk_buffer_.lo_) + ix;

                if ((tk_src <= p) && (p < tk_src + z))
                    ix += offset;
            }

            for (auto & ix : print_stack_) {
                const char * p = ((const char *)tk_buffer_.lo_) + ix;

                if ((tk_src <= p) && (p < tk_src + z))
                    ix += offset;
            }
        }

    } /*namespace print*/
} /*namespace xo*/

/* end PpState.cpp */
