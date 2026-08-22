/** @file AllocError.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AllocError.hpp"

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;

    namespace mm {

        const char *
        AllocError::error_description(error x)
        {
            switch (x) {
            case error::invalid:
                break;
            case error::ok:
                return "ok";
            case error::reserve_exhausted:
                return "reserve-exhausted";
            case error::commit_failed:
                return "commit-failed";
            case error::header_size_mask:
                return "header-size-mask";
            case error::orphan_sub_alloc:
                return "orphan-sub-alloc";
            case error::alloc_info_disabled:
                return "alloc-info-disabled";
            case error::alloc_info_address:
                return "alloc-info-address";
            case error::alloc_iterator_not_supported:
                return "alloc-iterator-not-supported";
            case error::alloc_iterator_deref:
                return "alloc-iterator-deref";
            case error::alloc_iterator_next:
                return "alloc-iterator-next";
            }

            return "?error";
        }

        void
        AllocError::pretty(PpSink & sink) const
        {
            sink.pretty_struct("AllocError",
                               field("error", error_),
                               field("src_fn", src_fn_, src_fn_ != nullptr),
                               field("seq", error_seq_),
                               field("req_z", request_z_),
                               field("commit_z", committed_z_),
                               field("resv_z", reserved_z_));
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end AllocError.cpp */
