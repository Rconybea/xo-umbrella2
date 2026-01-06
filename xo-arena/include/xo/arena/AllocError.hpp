/** @file AllocError.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <cstdint>
#include <cstddef>

namespace xo {
    namespace mm {
        enum class error : int32_t {
            /** sentinel **/
            invalid = -1,
            /** not an error **/
            ok,
            /** reserved size exhauged **/
            reserve_exhausted,
            /** unable to commit (i.e. mprotect failure) **/
            commit_failed,
            /** allocation size too big (See @ref ArenaConfig::header_size_mask_) **/
            header_size_mask,
            /** sub_alloc not preceded by super alloc (or another sub_alloc) **/
            orphan_sub_alloc,
            /** attempt to call alloc_info for allocator with alloc header feature disabled
             *  (e.g. @ref see ArenaConfig::store_header_flag_)
             **/
            alloc_info_disabled,
            /** attempt to call alloc_info for address not owned by allocator **/
            alloc_info_address,
            /** for example: alloc iteration not supported in arenas with
             *  AllocConfig.store_header_flag_ = false
             **/
            alloc_iterator_not_supported,
            /** attempt to deref an iterator that does not refer to an alloc **/
            alloc_iterator_deref,
            /** attempt to advance an iterator that does not refer to an alloc **/
            alloc_iterator_next,
        };

        struct AllocError {
            using size_type = std::size_t;
            using value_type = std::byte*;

            AllocError() = default;
            explicit AllocError(error err,
                                    uint32_t seq) : error_{err},
                                                    error_seq_{seq} {}
            AllocError(error err,
                           uint32_t seq,
                           size_type req_z,
                           size_type com_z,
                           size_type rsv_z) : error_{err},
                                              error_seq_{seq},
                                              request_z_{req_z},
                                              committed_z_{com_z},
                                              reserved_z_{rsv_z} {}

            static const char * error_description(error x);

            /** error code **/
            error error_ = error::ok;

            /** sequence# of this error.
             *  Each error event within an allocator gets next sequence number
             **/
            uint32_t error_seq_ = 0;
            /** reqeust size assoc'd with errror **/
            size_type request_z_ = 0;
            /** committed allocator memory at time of error **/
            size_type committed_z_ = 0;
            /** reserved allocator memory at time of error **/
            size_type reserved_z_ = 0;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end AllocError.hpp */
