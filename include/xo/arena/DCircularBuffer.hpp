/** @file DCircularBuffer.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "CircularBufferConfig.hpp"
#include "DArenaVector.hpp"
#include "hashmap/verify_policy.hpp"
#include "span.hpp"
#include <cstdint>

namespace xo {
    namespace mm {
        /** @class DCircularBuffer
         *
         *  @brief high performance vm-aware circular buffer
         *
         *  Circular buffer implementation with parsing-friendly performance features.
         *  - generalization of DArena.
         *    Like DArena, maps superpages as needed.
         *    Unlike DArena memory at the beginning of reserved range can be unmapped.
         *  - allows address range >> physical range
         *  - admits "Cheney on the MTA" strategy.
         *    May be feasible to reserve a lifetime address range (say 1TB)
         *    as long as buffer only every maps a subrange that fits in physical memory.
         *  - zero copy support for parsing / protocol trnaslation:
         *    provides capture/release semantics for a fixed number
         *    of remembered spans. Will never unmap memory for a remembered span,
         *    until that span is released.
         *  - automatically resets to beginning of reserved range
         *    whenever occupied range is empty
         **/
        struct DCircularBuffer {
        public:
            /** @defgroup mm-circularbuffer-types CircularBuffer type traits **/
            ///@{

            /** an amount of memory **/
            using size_type = std::size_t;
            using byte = std::byte;
            /** a contiguous addres range **/
            using span_type = span<char>;
            using const_span_type = span<const char>;

            ///@}

        public:
            /** @defgroup mm-cicrularbuffer-ctors CircularBuffer constructors **/
            ///@{

            /** contruct instance
             *  @p config          circular buffer configuration
             *  @p page_z          o/s page size (via getpagesize())
             *  @p buffer_align_z  alignment for buffer memory
             *  @p reserved_range  reserved virtual address range
             **/
            DCircularBuffer(const CircularBufferConfig & config,
                            size_type page_z,
                            size_type buffer_align_z,
                            span_type reserved_range);
#ifdef NOT_YET
            /** constructor */
            DCircularBuffer(const CircularBufferConfig & config);
#endif
            /** non-copyable **/
            DCircularBuffer(const DCircularBuffer & other) = delete;
            /** move ctor **/
            DCircularBuffer(DCircularBuffer && other);

            /**
             *  allocate virtual memory address (uncommitted!) for circular buffer
             *  with configuration @p config.
             **/
            static DCircularBuffer map(const CircularBufferConfig & config);

            ///@}

            /** @defgroup mm-circularbuffer-const-methods CircularBuffer const methods **/
            ///@{

            const_span_type reserved_range() const noexcept { return reserved_range_; }
            const_span_type   mapped_range() const noexcept { return mapped_range_; }
            const_span_type occupied_range() const noexcept { return occupied_range_; }
            const_span_type    input_range() const noexcept { return input_range_; }

            /** verify DCircularBuffer invariants.
             *  Act on failure according to policy @p p
             *  (combination of throw|log bits)
             *
             *  verify invariants:
             *  CB1: mapped_range_ is subrange of reserved_range_
             *  CB2: occupied_range_ is subrange of mapped_range_
             *  CB3: each remembered_spans_[i] is subrange of occupied_range_
             *  CB4: buffer_align_z_ > 0 when buffer is mapped
             *  CB5: reserved_range_.lo() aligned on buffer_align_z_ boundary
             **/
            bool verify_ok(verify_policy p = verify_policy::throw_only()) const;

            ///@}

            /** @defgroup mm-circularbuffer-nonconst-methods CircularBuffer non-const methods **/
            ///@{

            span_type input_range() noexcept { return input_range_; }

            /** copy memory in span @p r into buffer starting at the end of
             *  @ref occupied_range_. Map new physical memory as needed.
             *  On success returns empty suffix of @p r.
             *  If buffer memory exhausted, may copy a prefix of @p r.
             *  In that case returns the remaining suffix of @p r.
             **/
            const_span_type append(const_span_type r);

            /** DMA version of @ref append_span : get mapped span A at which
             *  buffer will receive new content. Upstream may write into
             *  A. It must then coordinate with buffer by calling
             *  @ref report_append(P) for some prefix P of A
             *
             *  Example:
             *  @code
             *    CircularBuffer buf = ...;
             *    constexpr size_type z = 64*1024;
             *    auto span = buf.get_append_span(z);
             *    ssize_t nr = read(FD, span.lo(), span.size());
             *    if (nr > 0)
             *        buf.report_append(span.prefix(nr));
             *  @endcode
             **/
            span_type get_append_span(size_type desired_z);

            /** update bookkeeping as if caller had invoked append(r);
             *  however caller has already written to mapped memory
             *  after using get_append_span(); so omit copy
             **/
            void report_append(span_type r);

            /** consume span (or prefix thereof) previously obtained from @ref occupied_range()
             *  Caller represents that it won't need to read this memory again
             *  unless overlaps with a pinned span.
             **/
            void consume(const_span_type input);

            /** pin memory range @p r. circular buffer will not touch
             *  addresses that appear in any pinned range.
             *  use to
             **/
            void pin_range(span_type r);

            /** unwind a previous pin_range call on range @p r.
             *  both start and end or @p r should exactly match a pinned range.
             **/
            void unpin_range(span_type r);

            ///@}

        private:

            /** @defgroup mm-circularbuffer-private-methods CircularBuffer non-const methods **/
            ///@{

            /** expand hi end of mapped memory range to at least @p hi.
             *
             *  Require: @p hi < @ref reserved_range_.hi
             **/
            bool _expand_to(char * hi);

            /** shrink occupied rnage to the smallest contiguous range that contains both:
             *  all of .input_range_, and all pinned ranges in .pinned_spans_
             **/
            void _shrink_occupied_to_fit();

            /** check for edge condition in which there are no pinned ranges. **/
            void _check_reset_map_start();

            ///@}

        private:
            /** @defgroup mm-circularbuffer-instance-vars CircularBuffer member variables **/
            ///@{

            /* memory layout
             *
             *  reserved_range_ : entire address range owned by buffer (may be huge, e.g., 1TB)
             *  mapped_range_   : subrange backed by physical memory (fits in RAM)
             *  occupied_range_ : subrange currently containing data
             *  input_range_    : subrange containing unread input
             *  pinned_spans_   : pinned subranges within occupied (prevents alteration or unmap)
             *
             *   <------------------- .reserved_range --------------------->
             *   .       <------------- .mapped_range ------------->       .
             *   .       .     <----- .occupied_range ----->       .       .
             *   .       .     .      <- .input_range ----->       .       .
             *   .       .     .      .                    .       .       .
             *   ........------XXXXXXXIIIIIIIIIIIIIIIIIIIIII--------........
             *                 pp       ppp      pp
             *  Legend:
             *    [.] reserved : uncommitted memory. may be huge (e.g. 1TB)
             *    [-] mapped   : range backed by physical memory
             *    [X] consumed : preserved until last overlapping pin removed
             *    [I] input    : unread content, waiting to be read
             *    [p] pinned   : pinned memory will not be altered (let alone unmapped)
             *
             *  Invariants:
             *  - .input_range <= .occupied_range <= .mapped_range <= .reserved_range
             *  - mapped_range_ cannot shrink to exclude any portion of a pinned span
             */

            /** buffer configuration **/
            CircularBufferConfig config_;

            /** size of a VM page (obtained automatically via getpagesize()). 4k on ubuntu. 16k on osx **/
            size_type page_z_;

            /** alignment for buffer address range.
             * In practice will be either page_z_ or config_.hugepage_z_
             **/
            size_type buffer_align_z_;

            /** Circular buffer owns address range defined by this span.
             *  Aligned on @ref buffer_align_z_.
             *  Always a whole number of @ref page_z_ or @ref config_.hugepage_z_
             **/
            span_type reserved_range_;

            /** buffer owns memory defined by this span.
             *  Always a subrange of reserved_range
             *  These addresses backed by physical memory.
             *  Always a whole number of @ref page_z_ or @ref config_.hugepage_z_
             **/
            span_type mapped_range_;

            /** currently occupied buffer memory.
             *  Always a subrange of @ref mapped_range_
             **/
            span_type occupied_range_;

            /** portion of occupied buffer memory waiting to be read.
             *  Always represents a subspan of @ref occupied_range_, with the same
             *  hi endpoint.
             *  conversely @ref consume shrinks @ref input_range_ by increasing its lo endpoint.
             **/
            span_type input_range_;

            /** remembered spans. For anticipated use cases expect one vm page sufficient.
             *  Spans in this vector always represent subranges of @ref occupied_range_
             *
             *  @ref pinned_spans_ is confined to @ref occupied_range_.
             *  (In particular it's *not* confined to @ref input_range_)
             *
             *  sorted on increasing span.lo()
             **/
            DArenaVector<span_type> pinned_spans_;

            ///@}
        };
    }
} /*namespace xo*/

/* end DCircularBuffer.hpp */
