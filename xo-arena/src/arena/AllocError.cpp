/** @file AllocError.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AllocError.hpp"

namespace xo {
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

    } /*namespace mm*/
} /*namespace xo*/

/* end AllocError.cpp */
