/** @file PpState.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpState.hpp"
#include "print/PpToken.hpp"
#include <xo/ppsink/escape.hpp>
#include <cassert>
#include <cstring>

namespace xo {
    using xo::mm::ArenaConfig;
    using std::max;
    using std::min;

    namespace pp {
        PpState::PpState(const PpLayoutConfig & layout)
            : layout_config_{layout},
              tk_buffer_{DArena::map(ArenaConfig()
                                     .with_name("pp-tk-buffer")
                                     .with_size((sizeof(PpStringToken) + 18)
                                                * layout.hard_right_margin()))},
              scan_stack_{ScanStack::map(ArenaConfig()
                                         .with_name("pp-scan-stack")
                                         .with_size(layout.hard_max_nesting()))},
              print_stack_{PrintStack::map(ArenaConfig()
                                           .with_name("pp-print-stack")
                                           .with_size(layout.hard_max_nesting()))}
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

            // relying on this because bypassing DArena::alloc()
            assert(tk_buffer_.config_.store_header_flag_ == false);
        }

        void
        PpState::connect_output(LogBuffer * p_out)
        {
            p_out_ = p_out;
        }

        void
        PpState::put_cstr(const char * c_str)
        {
            this->put(c_str);
        }

        void
        PpState::put(std::string_view str)
        {
            uint32_t c_str_z = str.size();
            uint32_t tk_z = PpStringToken::alloc_size(c_str_z);

            void * mem = this->alloc(tk_z);

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            uint32_t viz_z = count_visible_chars(str.begin(), str.end());
            uint32_t mem_z = tk_z - sizeof(PpStringToken);

            PpStringToken * tk
                = new (mem) PpStringToken(viz_z, c_str_z, mem_z, str.begin());

            scan_viz_total_ += tk->tk_viz_len();
            scan_total_ += tk->tk_len();

            // (note: noop unless scan stack is empty)
            this->check_print_ready();
        }

        void
        PpState::put_with_escape(std::string_view str, bool quote_flag)
        {
            // Escaping expands, so the token is not str.size() bytes: scan for
            // the expanded size first, then allocate, then expand into the
            // token.  Can't stream-and-grow, since alloc() places a
            // PpStringToken of known size.  Scanning also beats reserving
            // c_max_char_expand per character, which would drive tk_buffer_ to
            // expand for strings that don't need it.

            uint32_t esc_z = (Escape::str_size(str).size
                              + (quote_flag ? Escape::c_quote_expand : 0));
            uint32_t tk_z = PpStringToken::alloc_size(esc_z);

            void * mem = this->alloc(tk_z);

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            uint32_t mem_z = tk_z - sizeof(PpStringToken);

            // Build the token empty, then expand into its char array: unlike
            // put(), the bytes we mean to store don't exist anywhere yet for
            // the PpStringToken ctor to copy from.

            char empty_placeholder;

            PpStringToken * tk
                = new (mem) PpStringToken(0 /*viz_z*/,
                                          0 /*tk_len*/,
                                          mem_z,
                                          &empty_placeholder);

            char * lo = const_cast<char *>(tk->mem_span().lo());
            char * p = lo;

            if (quote_flag)
                *p++ = Escape::c_quote;

            p = Escape::str_copy(str, p);

            if (quote_flag)
                *p++ = Escape::c_quote;

            assert(static_cast<uint32_t>(p - lo) == esc_z);

            // ESC is always escaped, so count_visible_chars() cannot enter a
            // color-escape run over this range, making viz_z == esc_z.
            // Computed rather than assumed, so this stays correct if the
            // escape set changes, and so both put paths look alike.

            uint32_t viz_z = this->count_visible_chars(lo, p);

            // mem_z is already the padded fit for esc_z (we allocated exactly
            // alloc_size(esc_z)), so there is nothing to shrink here --
            // cf. commit_string(), which learns the size only afterwards.

            tk->finalize_inplace(viz_z, esc_z, mem_z);

            scan_viz_total_ += tk->tk_viz_len();
            scan_total_ += tk->tk_len();

            // (note: noop unless scan stack is empty)
            this->check_print_ready();
        }

        void
        PpState::begin(int32_t offset)
        {
            void * mem = this->alloc(sizeof(PpToken));

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            // Stash the scan totals as of this begin, so end() can recover the
            // group's own width as (scan totals then) - (scan totals now).
            //
            // Safe to borrow the size fields for this: size_established() keys
            // off the k_size_established flag (set only by establish_size()),
            // and check_print_ready() stops at the first token whose size is not
            // established -- so nothing reads these as a *size* before end()
            // overwrites them.
            PpToken * tk = new (mem) PpToken(k_begin,
                                             (int32_t)scan_viz_total_ /*snapshot*/,
                                             (int32_t)scan_total_ /*snapshot*/,
                                             offset);
            uint32_t tk_ix = (std::byte *)mem - tk_buffer_.lo_;

            scan_stack_.push_back(tk_ix);

            // begin-token cannot trigger printing,
            // since doesn't carry any size information.

            (void)tk;
            assert(tk);
        }

        void
        PpState::split(uint32_t spaces, int32_t offset)
        {
            void * mem = this->alloc(sizeof(PpToken));

            if (!mem) [[unlikely]] {
                assert(false);
                return;
            }

            PpToken * tk = new (mem) PpToken(k_split,
                                             (int32_t)spaces /*viz_len*/,
                                             (int32_t)spaces /*len*/,
                                             offset);

            (void)tk;
            assert(tk);

            // the flat-form spaces count toward the enclosing group's width,
            // so a group with a `spaces`-carrying split is measured correctly.
            scan_viz_total_ += spaces;
            scan_total_ += spaces;

            // split size is known immediately, but a split never unblocks
            // printing on its own (only a matching end can), so no
            // check_print_ready() here -- matches the historical behavior.
        }

        void
        PpState::newline(int32_t offset)
        {
            // forced break: mark every currently-open group as must-break,
            // then emit a zero-space split.  Since the enclosing group can no
            // longer "fit", that split will render as newline+indent.
            for (auto & ix : scan_stack_) {
                PpToken * b = (PpToken *)((char *)tk_buffer_.lo_ + ix);
                b->set_forced_flag();
            }

            this->split(0 /*spaces*/, offset);
        }

        void
        PpState::write_spaces(uint32_t n)
        {
            static const char s_space = ' ';

            for (uint32_t i = 0; i < n; ++i)
                p_out_->write_span(xo::mm::span<const char>(&s_space, &s_space + 1));
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
            assert(tk);

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
            //
            // NB measured against the scan totals SNAPSHOTTED BY begin() (stashed
            // in the token's size fields), not against print_viz_total_.
            //
            // print_viz_total_ is how much has been *printed*, which says nothing
            // about where this group started -- and printing is deferred until a
            // group's fate is known, so it is typically far behind.  Using it made
            // a group's measured width include everything scanned before the group
            // even opened.  The deeper the nesting, the more foreign content was
            // counted, so inner groups reported far too wide and broke when they
            // would have fit.  Symptoms: a record whose flat form was well inside
            // the margin still broke; identical siblings laid out differently; and
            // output that stopped responding to the margin once everything had
            // bottomed out.
            const uint32_t begin_viz_total = (uint32_t)begin_token->tk_viz_len();
            const uint32_t begin_total = (uint32_t)begin_token->tk_len();

            if ((scan_viz_total_ < begin_viz_total)
                || (scan_total_ < begin_total)) [[unlikely]] {

                assert(false);
                return;
            }

            uint32_t tk_viz_z = scan_viz_total_ - begin_viz_total;
            uint32_t tk_z = scan_total_ - begin_total;

            begin_token->establish_size(tk_viz_z, tk_z);

            this->check_print_ready();
        }

        auto
        PpState::open_string(uint32_t min_z) -> Span
        {
            assert(!this->has_open_string());

            uint32_t alloc_z = PpStringToken::alloc_size(min_z);
            uint32_t tk_mem = alloc_z - sizeof(PpStringToken);

            void * header = this->alloc(alloc_z);
            assert(header);

            // counts whatever space is available (possible zero)
            // _after_ allocation

            uint32_t extra_z = this->available();

            // Keep token capacity padded, given available must leave one buffer
            // space unused to avoid confusing {empty,full} buffer
            extra_z -= (extra_z % sizeof(uint32_t));

            char empty_placeholder;

            // PpState will have no available space until open string is committed.
            this->scan_ix_ += extra_z;

            PpStringToken * s
                = new (header) PpStringToken(0 /*viz_z*/,
                                             0 /*size*/,
                                             tk_mem + extra_z,
                                             &empty_placeholder);

            assert((const char *)tk_buffer_.lo_ + scan_ix_ == s->span().lo() + s->tk_mem());

            this->current_open_string_ = s;

            return Span(const_cast<char *>(s->mem_span().lo()),
                        const_cast<char *>(s->mem_span().hi()));
        }

        void
        PpState::commit_string(Span used)
        {
            assert(this->has_open_string());

            PpStringToken * s = current_open_string_;

            assert(s);
            assert(used.lo() == s->mem_span().lo());
            assert(used.hi() <= s->mem_span().hi());

            // verify scan_ix_ hasn't been
            assert((const char *)tk_buffer_.lo_ + scan_ix_ == s->span().lo() + s->tk_mem());

            // now that we know actual string span, shrink to padded-fit

            uint32_t tk_len = used.size();
            uint32_t tk_viz_len = this->count_visible_chars(used.lo(), used.hi());
            uint32_t tk_mem = PpStringToken::alloc_size(tk_len) - sizeof(PpStringToken);

            assert(tk_mem <= s->tk_mem());

            s->finalize_inplace(tk_viz_len, tk_len, tk_mem);

            // retreat scan_ix_ based on now-known actual size of s
            this->scan_ix_ = s->mem_span().hi() - (const char *)tk_buffer_.lo_;
            this->scan_viz_total_ += tk_viz_len;
            this->scan_total_ += tk_len;

            this->current_open_string_ = nullptr;

            // (note: noop unless scan stack is empty)
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

            if (extent_ && (print_ix_ == extent_)) [[unlikely]] {
                // dead. could unwrap as per below, but want to know
                assert(false);

                extent_ = 0;
                print_ix_ = 0;
            }

            while (print_ix_ != scan_ix_) {
                PpToken * token = (PpToken *)((char *)tk_buffer_.lo_ + print_ix_);

                if (!token->size_established()) {
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

                        bool f = ((lpos + token->tk_viz_len()
                                   < layout_config_.soft_right_margin())
                                  && !token->is_forced());

                        token->set_fits_flag(f);
                        print_stack_.push_back(print_ix_);
                        print_indent_ += token->tk_offset();
                    }
                    break;
                case k_split:
                    {
                        /* split with no enclosing group: nothing constrains it,
                         * so treat like a split in a group that fits (no-op)
                         */
                        if (print_stack_.empty())
                            break;

                        uint32_t parent_ix = print_stack_.back();
                        PpToken * parent = (PpToken *)((char *)tk_buffer_.lo_ + parent_ix);

                        if (parent->tk_flags() & k_fits) {
                            // parent fits: render the break as its flat spaces
                            uint32_t spaces = (uint32_t)token->tk_viz_len();
                            this->write_spaces(spaces);
                            print_viz_total_ += spaces;
                            print_total_ += spaces;
                        } else {
                            // parent doesn't fit: break is newline + indent,
                            // where indent = running indent + this split's offset
                            int32_t off = print_indent_ + token->tk_offset();
                            uint32_t indent_z = (off > 0) ? (uint32_t)off : 0;

                            p_out_->newline_indent(indent_z);
                            print_viz_total_ += (1 + indent_z);
                            print_total_ += (1 + indent_z);
                        }
                    }
                    break;
                case k_end:
                    if (!print_stack_.empty()) {
                        uint32_t begin_ix = print_stack_.back();
                        PpToken * begin_tk
                            = (PpToken *)((char *)tk_buffer_.lo_ + begin_ix);
                        print_indent_ -= begin_tk->tk_offset();
                        print_stack_.pop_back();
                    }
                    break;
                }

                print_ix_ += token->alloc_size();

                if (print_ix_ == extent_) {
                    extent_ = 0;    // discard buffer wrap, no longer needed
                    print_ix_ = 0;
                }
            }

            if (extent_ && (print_ix_ == extent_)) [[unlikely]] {
                // could unwrap here, but should be unreachable
                assert(false);

                extent_ = 0;
                print_ix_ = 0;
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
                    } else if (isdigit(*p) || (*p == '[') || (*p == ';')) {
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
        PpState::alloc_scan_aux(uint32_t z)
        {
            void * retval = (char *)tk_buffer_.lo_ + scan_ix_;
            scan_ix_ += z;

            // cosmetic: get arena free pointer to extend to upper
            // orbit of scan_ix_
            if ((tk_buffer_.free_ < tk_buffer_.lo_ + scan_ix_)
                && (retval < tk_buffer_.limit_))
            {
                tk_buffer_.free_ = tk_buffer_.lo_ + scan_ix_;
            }

            return retval;
        }

        void *
        PpState::alloc(uint32_t z)
        {
            if (print_ix_ <= scan_ix_) {
                // buffer currently in one segment [print_ix_, scan_ix_)

                uint32_t avail1_z = tk_buffer_.committed() - scan_ix_;

                if (z <= avail1_z) {
                    return this->alloc_scan_aux(z);
                } else if (z < print_ix_) {
                    if (print_ix_ == scan_ix_) {
                        // buffer is empty -> restart at 0
                        print_ix_ = 0;
                    } else {
                        // wrap: start 2nd segment in [tk_buffer_.lo_, print_ix_)
                        extent_ = scan_ix_;
                    }
                    scan_ix_ = z;

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
            {
                auto grow_z = std::max(z, scan_ix_);

                // expand buffer to make room.
                // If it's wrapped, we're going to insist on unwrapping it.

                if (!tk_buffer_.expand(tk_buffer_.committed() + grow_z, "PpState::expand")) {
                    assert(false);
                    return nullptr;
                }
            }

            if (print_ix_ <= scan_ix_) {
                if (scan_ix_ + z <= tk_buffer_.committed()) [[likely]] {
                    return this->alloc_scan_aux(z);
                }
                // fall through
            } else {
                // 1. buffer is currently split into two segments
                //      [print_ix, extent) and [0, scan_ix_).
                // 2. new memory (reflected in avail_z) has appeared
                //    /after/ [prinx_ix, extent) so logically between the two segments.
                //    Use that new memory to paste the two segments back together

                uint32_t avail_z = tk_buffer_.committed() - extent_;
                uint32_t copy_z = min(scan_ix_, avail_z);

                ::memcpy(tk_buffer_.lo_ + extent_,
                         tk_buffer_.lo_,
                         copy_z);

                this->reindex_stacks(((const char *)tk_buffer_.lo_) + extent_,
                                     (const char *)tk_buffer_.lo_,
                                     copy_z);

                scan_ix_ = extent_ + copy_z;

                if (scan_ix_ + z < tk_buffer_.committed()) {
                    extent_ = 0; // buffer now unwrapped

                    return this->alloc_scan_aux(z);
                } else if (z < print_ix_) {
                    // alloc(z) needs to re-introduce wrap
                    // with 1st segment
                    //   [print_ix, extent_)
                    // and 2nd segment
                    //   [0, scan_ix_) = [0, z)

                    // wrap: start 2nd segment in [0, print_ix_)
                    extent_ = scan_ix_;
                    scan_ix_ = z;

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

        void
        PpState::visit_pools(const MemorySizeVisitor & fn) const
        {
            tk_buffer_.visit_pools(fn);
            scan_stack_.visit_pools(fn);
            print_stack_.visit_pools(fn);
        }

    } /*namespace pp*/
} /*namespace xo*/

/* end PpState.cpp */
