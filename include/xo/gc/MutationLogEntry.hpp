/** @file MutationLogEntry.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/alloc2/GCObject.hpp>

namespace xo {
    namespace mm {

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
            MutationLogEntry(void * parent, void ** p_data, obj<AGCObject> snap);

        private:
            /** address of object containing logged mutation **/
            void * parent_ = nullptr;
            /** address of target member of object at address @ref parent_,
             *  driving this log entry.
             **/
            void ** p_data_ = nullptr;
            /** AGCObject i/face pointer, asof assignment responsible for this log entry.
             *  If *p_data_ matches snap_.data(), then AGCObject interface is snap_.iface().
             *  Otherwise log entry has been superseded by another assignment.
             **/
            obj<AGCObject> snap_;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogEntry.hpp */
