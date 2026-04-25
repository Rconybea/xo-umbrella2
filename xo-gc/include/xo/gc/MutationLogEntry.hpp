/** @file MutationLogEntry.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/alloc2/GCObject.hpp>

namespace xo {
    namespace mm {
        class GCObjectStore;  // see xo-gc/ GCObjectStore.hpp
        class MutationLogStatistics; // see xo-gc/ MutationLogStatistics.hpp

        /** @brief Track a cross-generational pointer
         *
         *  GC must update pointer when collecting space that target occupies
         *
         *  Design notes:
         *  - parent must be located at the beginning of an allocation,
         *    (so that it's immediately preceded by allocation header)
         *  - destination can be something like
         *      obj<AGCObject>
         *    but also something else such as
         *      {obj<AType>, obj<ASyntaxStateMachine>, ..}
         *  - for collector need to traverse data pointer *data
         **/
        class MutationLogEntry {
        public:
            using AGCObject = xo::mm::AGCObject;

        public:
            MutationLogEntry() = default;
            MutationLogEntry(void * parent, void ** p_data, obj<AGCObject> snap);

            void * parent() const { return parent_; }
            void ** p_data() const { return p_data_; }
            obj<AGCObject> snap() const { return snap_; }

            /** true iff child pointer matches value when this mlog entry created **/
            bool is_active() const noexcept { return *p_data_ == snap_.data(); }
            /** true iff child pointer has been altered since this mlog entry created
             *  WARNING: extra care needed around forwarding pointers
             **/
            bool is_superseded() const noexcept { return *p_data_ != snap_.data(); }

            /** Update parent and snapshot if either has been forwarded.
             *  Do not try to correct *p_data_: it might now point outside
             *  gc-owned space, in which case need not be intelligible
             **/
            bool check_forward_inplace(GCObjectStore * gcos,
                                       MutationLogStatistics * p_counters) noexcept;

            /** update @ref parent_, @ref p_data_ iff parent has been forwarded
             *  @return true iff encountered forwarded parent
             *  Require: parent must be gc-owned, since we rely on AllocInfo existing
             **/
            bool check_forward_parent_inplace(GCObjectStore * gcos,
                                              MutationLogStatistics * p_counters) noexcept;

            /** Refresh snapshot when *p_data_ does not match snap_.data_
             *  Get updated facet information from destination alloc header.
             *  It's possible that *p_data_ no longer points to gc-owned space
             *
             *  @return true if snapshot updated. false if this entry should be discarded
             **/
            bool refresh_snapshot(Generation parent_gen,
                                  GCObjectStore * gcos) noexcept;

        private:
            /** address of object containing logged mutation **/
            void * parent_ = nullptr;
            /** address of target member of object at address @ref parent_,
             *  driving this log entry.
             **/
            void ** p_data_ = nullptr;
            /** AGCObject child pointer, asof assignment responsible for this log entry.
             *  If *p_data_ matches snap_.data(), then AGCObject interface is snap_.iface().
             *  Otherwise log entry has been superseded by another assignment.
             **/
            obj<AGCObject> snap_;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogEntry.hpp */
