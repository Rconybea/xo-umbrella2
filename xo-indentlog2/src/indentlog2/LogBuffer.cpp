/** @file LogBuffer.cpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#include "LogBuffer.hpp"
#include <streambuf>
#include <cstring>

namespace xo {
    using xo::mm::ArenaConfig;
    using std::size_t;

    LogBuffer::LogBuffer(const ArenaConfig & config, bool debug_flag)
        : buf_v_{CharBuffer::map(config)},
          porigin_{nullptr}, bpptr_{nullptr}, pptr_{nullptr}, epptr_{nullptr}, lstate_{},
          debug_flag_{debug_flag}
    {
        // buf_v_ always contains one allocation, comprising its entire size

        /* ask for 256 byte, in practice will get one vm page
         * However, available memory less than that if we used object header
         */
        this->expand_to(256);
    }

    auto
    LogBuffer::committed_span() const -> Span
    {
        return Span(porigin_, epptr_);
    }

    auto
    LogBuffer::used_span() const -> Span
    {
        return Span(porigin_, pptr_);
    }

    auto
    LogBuffer::available_span() const -> Span
    {
        return Span(pptr_, epptr_);
    }

    void
    LogBuffer::visit_pools(const MemorySizeVisitor & fn) const {
        buf_v_.visit_pools(fn);
    }

    bool
    LogBuffer::expand_to(size_t new_z)
    {
        bool ok = buf_v_.expand(new_z, "LogBuffer::expand_to");

        if (!ok)
            return false;

        if (porigin_ == nullptr) [[unlikely]] {
            // first call -> establish porigin_,
            // (arena may prefix with guard bytes / alloc header
            // so possible that available < committed

            auto z = buf_v_.available();

            buf_ckp_ = buf_v_.checkpoint();
            porigin_ = (char *)buf_v_.alloc(reflect::typeseq::id<char[]>(), z);

            bpptr_ = porigin_;
            pptr_ = porigin_;
            epptr_ = porigin_ + z;
        } else {
            /* restore to checkpoint, then allocate.
             * This is workaround for missing DArena::realloc().
             * (+ only need checkpoint if DArena configured with object headers)
             */
            buf_v_.restore(buf_ckp_);

            auto z = buf_v_.available();

            auto porigin0 = porigin_;

            porigin_ = (char *)buf_v_.alloc(reflect::typeseq::id<char[]>(), z);

            ok = ok && (porigin_ == porigin0);

            // keep bpptr_/pptr_, we reallocated in place
            epptr_ = porigin_ + z;
        }

        return ok;
    }

    bool
    LogBuffer::_require_avail(uint32_t x)
    {
        Span out = this->available_span();

        if (x > out.size()) {
            auto min_z = this->used_span().size() + x;

            // expand to make room
            if (!this->expand_to(min_z)) {
                return false;
            }

            out = this->available_span();
        }

        return true;

    }

    void
    LogBuffer::newline_indent(uint32_t indent)
    {
        if (!this->_require_avail(1 + indent)) {
            assert(false);
            return;
        }

        auto pptr = this->pptr_;

        *(pptr++) = '\n';
        ::memset(pptr, ' ', indent);
        pptr += indent;

        this->_check_update_local_state(pptr);
    }

    void
    LogBuffer::write_span(ConstSpan x)
    {
        if (!this->_require_avail(x.size())) {
            assert(false);
            return;
        }

        // here: buffer has enough room now

        auto pptr = pptr_;
        ::memcpy(pptr, x.lo(), x.size());
        pptr += x.size();

        this->_check_update_local_state(pptr);
    }

    void
    LogBuffer::flush()
    {
        if (dest_ && (bpptr_ < pptr_)) {
            dest_->sputn(bpptr_, pptr_ - bpptr_);
            bpptr_ = pptr_;
        }
    }

    void
    LogBuffer::reset_buffer()
    {
        // scrub buffered chars? perhaps offer as option
        // keep {porigin_, epptr_}: allocated extent not changed

        // drain pending output before reclaiming the memory holding it,
        // and push it through to the device (record boundary = sync point)
        this->flush();
        if (dest_)
            dest_->pubsync();

        if (porigin_) {
            buf_v_.restore(buf_ckp_);
        }
        this->bpptr_ = porigin_;
        this->pptr_ = porigin_;

        lstate_.clear();
    }

    void
    LogBuffer::_check_update_local_state(char * pptr)
    {
        lstate_._check_update_local_state(porigin_, pptr, debug_flag_);
        this->pptr_ = pptr;
    }
} /*namespace xo*/

/* end LogBuffer.cpp */
