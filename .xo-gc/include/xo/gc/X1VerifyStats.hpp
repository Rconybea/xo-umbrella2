/** @file X1VerifyStats.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    namespace mm {

        /** @brief info collected during a @ref DX1Collector::verify_ok call
         *  (or @ref MutationLogState::verify_ok call)
         **/
        class X1VerifyStats {
        public:
            bool is_ok() const noexcept {
                return (n_from_ == 0) && (n_fwd_ == 0) && (n_age_bad_ == 0) && (n_no_iface_ == 0);
            }

            void clear() { *this = X1VerifyStats(); }

            /** number of gc roots examined **/
            std::uint32_t n_gc_root_  = 0;
            std::uint32_t n_ext_      = 0;
            /** number of from-space objects encountered. Fatal if non-zero **/
            std::uint32_t n_from_     = 0;
            /** number of to-space objects encountered. **/
            std::uint32_t n_to_       = 0;
            /** counts forwarding object encountered in to-space scan. Fatal if non-zero **/
            std::uint32_t n_fwd_      = 0;
            /** counts objects in expected generation for age **/
            std::uint32_t n_age_ok_   = 0;
            /** counts objects in wrong generation for age **/
            std::uint32_t n_age_bad_  = 0;
            /** counts missing GCObject interface. Fatal if non-zero **/
            std::uint32_t n_no_iface_ = 0;
            /** live mlog entry refers to to-space, as expected **/
            std::uint32_t n_mlog_vital_ = 0;
            /** stale mlog entry. not troubling to verify these **/
            std::uint32_t n_mlog_stale_ = 0;
            /** live mlog entry refers to from-space. Fatal if non-zero **/
            std::uint32_t n_mlog_from_ = 0;
            /** live mlog entry refers to either some other generation or outside gc-space.
             *  Fatal if non-zero
             **/
            std::uint32_t n_mlog_wild_ = 0;

        };

    } /*namespace mm*/
} /*namespace xo*/

/* end X1VerifyStats.hpp */
