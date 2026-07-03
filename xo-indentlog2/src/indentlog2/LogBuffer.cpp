/** @file LogBuffer.cpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#include "LogBuffer.hpp"

namespace xo {
    using xo::mm::ArenaConfig;
    using std::size_t;

    LogBuffer::LogBuffer(const ArenaConfig & config, bool debug_flag)
        : buf_v_{CharBuffer::map(config)},
          pbase_{nullptr}, epptr_{nullptr}, lstate_{},
          debug_flag_{debug_flag}
    {
        // buf_v_ always contains one allocation, comprising its entire size

        /* ask for 256 byte, in practice will get one vm page
         * However, available memory less than that if we used object header
         */
        this->expand_to(256);
    }

    auto
    LogBuffer::committed_span() -> Span
    {
        return Span(pbase_, epptr_);
    }

    void
    LogBuffer::visit_pools(const MemorySizeVisitor & fn) const {
        buf_v_.visit_pools(fn);
    }

    bool
    LogBuffer::expand_to(size_t new_z)
    {
        bool ok = buf_v_.expand(new_z, "LogBuffer::expand_to");
        assert(ok);

        if (pbase_ == nullptr) [[unlikely]] {
            // first call -> establish pbase_,
            // (arena may prefix with guard bytes / alloc header
            // so possible that available < committed

            auto z = buf_v_.available();

            buf_ckp_ = buf_v_.checkpoint();
            pbase_ = (char *)buf_v_.alloc(reflect::typeseq::id<char[]>(), z);

            epptr_ = pbase_ + z;
        } else {
            /* restore to checkpoint, then allocate */
            buf_v_.restore(buf_ckp_);

            auto z = buf_v_.available();

            auto pbase0 = pbase_;

            pbase_ = (char *)buf_v_.alloc(reflect::typeseq::id<char[]>(), z);

            ok = ok && (pbase_ == pbase0);
            assert(ok);

            epptr_ = pbase_ + z;
        }

        return ok;
    }

    void
    LogBuffer::reset_buffer()
    {
        // scrub buffered chars? perhaps offer as option
        // keep {pbase_, epptr_}: allocated extent not changed

        lstate_.clear();
    }

    void
    LogBuffer::_check_update_local_state(const char * pptr)
    {
        lstate_._check_update_local_state(pbase_, pptr, debug_flag_);
    }
} /*namespace xo*/

/* end LogBuffer.cpp */
