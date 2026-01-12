/** @file DCircularBuffer.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DCircularBuffer.hpp"
#include "mmap_util.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tostr.hpp>
#include <sys/mman.h>

namespace xo {
    using xo::print::operator<<;
    using xo::print::printspan;

    namespace mm {

        DCircularBuffer::DCircularBuffer(DCircularBuffer && other)
        : config_{other.config_},
          page_z_{other.page_z_},
          buffer_align_z_{other.buffer_align_z_},
          reserved_range_{other.reserved_range_},
          mapped_range_{other.mapped_range_},
          occupied_range_{other.occupied_range_},
          pinned_spans_{std::move(other.pinned_spans_)}
        {
            other.reserved_range_ = span_type();
            other.mapped_range_ = span_type();
            other.occupied_range_ = span_type();
        }

        DCircularBuffer
        DCircularBuffer::map(const CircularBufferConfig & config)
        {
            scope log(XO_DEBUG(config.debug_flag_));

            /* vm page size. 4KB (probably if linux) or 16KB (probably if osx) */
            size_t page_z = getpagesize();

            bool enable_hugepage_flag = (config.max_capacity_ >= config.hugepage_z_);

            /* Align start of arena memory on this boundary.
             * Will use THP (transparent huge pages) if available
             * and arena size is at least as large as hugepage size (2MB, probably)
             */
            size_t align_z = (enable_hugepage_flag ? config.hugepage_z_ : page_z);

            log && log(xtag("page_z", page_z),
                       xtag("align_z", align_z));

            auto mapped_span
                = span<char>::from_memory(mmap_util::map_aligned_range
                                              (config.max_capacity_,
                                               align_z,
                                               enable_hugepage_flag,
                                               config.debug_flag_));

            if (!mapped_span.lo()) {
                throw std::runtime_error(tostr("DCircularBuffer: reserve address range failed",
                                               xtag("size", config.max_capacity_)));
            }

            return DCircularBuffer(config, page_z, align_z, mapped_span);
        }

        DCircularBuffer::DCircularBuffer(const CircularBufferConfig & config,
                                         size_type page_z,
                                         size_type buffer_align_z,
                                         span_type reserved_range)
        : config_{config},
          page_z_{page_z},
          buffer_align_z_{buffer_align_z},
          reserved_range_{reserved_range},
          mapped_range_{reserved_range_.prefix(0)},
          occupied_range_{mapped_range_.prefix(0)},
          input_range_{occupied_range_.prefix(0)},
          pinned_spans_{}
        {
        }

        bool
        DCircularBuffer::verify_ok(verify_policy policy) const
        {
            using xo::xtag;

            constexpr const char * c_self = "DCircularBuffer::verify_ok";
            scope log(XO_DEBUG(false));

            /* CB1: mapped_range_ is subrange of reserved_range_ */
            if ((mapped_range_.lo() < reserved_range_.lo())
                || (mapped_range_.hi() > reserved_range_.hi()))
            {
                return policy.report_error(log,
                                           c_self, ": expect mapped_range subset of reserved_range",
                                           xtag("mapped.lo", (void*)mapped_range_.lo()),
                                           xtag("mapped.hi", (void*)mapped_range_.hi()),
                                           xtag("reserved.lo", (void*)reserved_range_.lo()),
                                           xtag("reserved.hi", (void*)reserved_range_.hi()));
            }

            /* CB2: occupied_range_ is subrange of mapped_range_ */
            if ((occupied_range_.lo() < mapped_range_.lo())
                || (occupied_range_.hi() > mapped_range_.hi()))
            {
                return policy.report_error(log,
                                           c_self, ": expect occupied_range subset of mapped_range",
                                           xtag("occupied.lo", (void*)occupied_range_.lo()),
                                           xtag("occupied.hi", (void*)occupied_range_.hi()),
                                           xtag("mapped.lo", (void*)mapped_range_.lo()),
                                           xtag("mapped.hi", (void*)mapped_range_.hi()));
            }

            /* CB3: each remembered span is subrange of occupied_range_ */
            for (size_type i = 0, n = pinned_spans_.size(); i < n; ++i) {
                const const_span_type & pin = pinned_spans_[i];

                if ((pin.lo() < occupied_range_.lo())
                    || (pin.hi() > occupied_range_.hi()))
                {
                    return policy.report_error(log,
                                               c_self, ": expect remembered_span subset of occupied_range",
                                               xtag("i", i),
                                               xtag("pin.lo", (void*)pin.lo()),
                                               xtag("pin.hi", (void*)pin.hi()),
                                               xtag("occupied.lo", (void*)occupied_range_.lo()),
                                               xtag("occupied.hi", (void*)occupied_range_.hi()));
                }
            }

            /* CB4: buffer_align_z_ is non-zero (when buffer is mapped) */
            if (!reserved_range_.is_null() && (buffer_align_z_ == 0)) {
                return policy.report_error(log,
                                           c_self, ": expect buffer_align_z > 0 when buffer is mapped",
                                           xtag("buffer_align_z", buffer_align_z_));
            }

            /* CB5: reserved_range_ aligned on buffer_align_z_ boundary */
            if (!reserved_range_.is_null() && (buffer_align_z_ > 0)) {
                if (((size_type)(reserved_range_.lo()) % buffer_align_z_) != 0) {
                    return policy.report_error(log,
                                               c_self, ": expect reserved_range.lo aligned on buffer_align_z",
                                               xtag("reserved.lo", (void*)reserved_range_.lo()),
                                               xtag("buffer_align_z", buffer_align_z_));
                }
            }

            return true;
        }

        auto
        DCircularBuffer::append(const_span_type src) -> const_span_type
        {
            span_type dest = get_append_span(src.size());

            size_t copy_z = std::min(src.size(), dest.size());

            ::memcpy(occupied_range_.hi(), src.lo(), copy_z);

            this->occupied_range_ += span_type(dest.lo(), copy_z);
            this->input_range_ += span_type(dest.lo(), copy_z);

            return src.after_prefix(copy_z);
        }

        auto
        DCircularBuffer::get_append_span(size_type desired_z) -> span_type
        {
            span_type dest = span_type(occupied_range_.hi(), desired_z);

            if (dest.hi() > reserved_range_.hi()) {
                /* under no circumstances go past the end of reserved range */
                dest = span_type(dest.lo(), reserved_range_.hi());
            }

            /* establish mapped range at least to dest.hi */
            this->_expand_to(dest.hi());

            /* report available memory */
            return span_type(occupied_range_.hi(), mapped_range_.hi());
        }

        void
        DCircularBuffer::report_append(span_type r)
        {
            if (r.lo() != occupied_range_.hi()) {
                // error!

                // this->capture_error(error::bad_append_report, r.size())
                assert(false);

                return;
            }

            if (r.hi() > mapped_range_.hi()) {
                // error!

                // this->capture_error(error::bad_append_report, r.size())
                assert(false);

                return;
            }

            this->occupied_range_ += r;
        }

        void
        DCircularBuffer::consume(const_span_type input)
        {
            scope log(XO_DEBUG(false), xtag("input", input.to_string_view()));

            if (input.lo() != input_range_.lo()) {
                assert(false);

                return;
            }

            if (input.hi() > occupied_range_.hi()) {
                assert(false);

                return;
            }

            if (occupied_range_.lo() < input_range_.lo()) {
                log && log("pinned range prevents shrinking occupied range");

                /* here: a pinned range prevents shrinking occupied_range */

                this->input_range_
                    = input_range_.suffix_from((span_type::value_type *)input.hi());
            } else {
                log && log(xtag("msg", "will shrink occupied range"),
                           xtag("input.lo", (void*)input.lo()),
                           xtag("input.hi", (void*)input.hi()),
                           xtag("stored.lo", (void*)input_range_.lo()),
                           xtag("stored.hi", (void*)input_range_.hi())
                           );

                /* here: input; recompute occupied boundary */

                this->input_range_
                    = input_range_.suffix_from((span_type::value_type *)input.hi());

                log && log(xtag("occupied", occupied_range_.size()),
                           xtag("input", input_range_.size()));

                this->_shrink_occupied_to_fit();

                log && log(xtag("occupied", occupied_range_.size()),
                           xtag("input", input_range_.size()));
            }

            this->_check_reset_map_start();
        }

        void
        DCircularBuffer::pin_range(span_type r)
        {
            // loop optimized for case where r falls
            // _after_ any existing pinned ranges

            size_type z = pinned_spans_.size();
            size_type ip1 = z;  // ip1 = i + 1

            for (; ip1 > 0; --ip1) {
                if (r.lo() > pinned_spans_[ip1 - 1].lo())
                    break;

                // insert at i to maintain sorted order
                pinned_spans_.insert(ip1 - 1, r);
                return;
            }

            pinned_spans_.push_back(r);
        }

        void
        DCircularBuffer::unpin_range(span_type r)
        {
            // loop optimized for case where r
            // is the first pinned range

            assert(pinned_spans_.size() > 0);

            if (r == pinned_spans_[0]) {
                this->pinned_spans_.erase(0);

                /* removing pinned span means can perhaps shrink
                 * occupied range
                 */
                this->_shrink_occupied_to_fit();
                this->_check_reset_map_start();
            } else {
                for (size_type i = 1; i < pinned_spans_.size(); ++i) {
                    if (r == pinned_spans_[i]) {
                        this->pinned_spans_.erase(i);

                        /* since this isn't the first pinned span,
                         * won't be able to shrink occupied range.
                         */
                        return;
                    }
                }
            }
        }

        bool
        DCircularBuffer::_expand_to(char * hi)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            if (hi < mapped_range_.hi()) {
                /* nothing todo */
                return true;
            }

            size_t add_z = hi - mapped_range_.hi();
            size_t add_commit_z = padding::with_padding(add_z, buffer_align_z_);
            char * commit_start = mapped_range_.hi();

            if (::mprotect(commit_start,
                           add_commit_z,
                           PROT_READ | PROT_WRITE) != 0)
                {
                    if (log) {
                        log("commit failed");
                        log(xtag("commit_start", commit_start),
                            xtag("add_z", add_z),
                            xtag("add_commit_z", add_commit_z));
                    }

                    // this->capture_error(error::commit_failed, add_commit_z);
                    return false;
                }

            this->mapped_range_ += span(commit_start, add_commit_z);
            return true;
        }

        void
        DCircularBuffer::_shrink_occupied_to_fit()
        {
            if (pinned_spans_.empty()) {
                this->occupied_range_ = input_range_;
            } else if (occupied_range_.lo() < pinned_spans_[0].lo()) {
                this->occupied_range_ = occupied_range_.suffix_from(pinned_spans_[0].lo());
            }
        }

        void
        DCircularBuffer::_check_reset_map_start()
        {
            if (pinned_spans_.empty()
                && (input_range_ == occupied_range_)) {

                // here: permissible to move input range to the beginning of mapped range.
                // decide (heuristically) whether we think this is optimal

                std::size_t input_z = input_range_.size();

                // 1st clause checks efficiency.
                // 2nd clause (probably redundant) check non-overlapping
                if ((input_range_.lo() > (mapped_range_.lo()
                                          + std::max(page_z_,
                                                     static_cast<size_type>(config_.threshold_move_efficiency_ * input_z))))
                    && (mapped_range_.lo() + input_z < input_range_.lo())) {

                    ::memmove(mapped_range_.lo(), input_range_.lo(), input_z);

                    this->occupied_range_ = mapped_range_.prefix(input_z);
                    this->input_range_ = mapped_range_.prefix(input_z);
                }
            }
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DCircularBuffer.cpp */
