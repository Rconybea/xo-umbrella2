/** @file WsSessionTable.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>

namespace xo {
    namespace web {
        /** @brief a webserver's live websocket sessions, by id.
         *
         *  Session ids come from a counter and are NEVER reused: once a
         *  session closes, its id addresses nothing, ever.  So anything still
         *  holding a closed session's id -- e.g. a sink the application kept --
         *  can never reach a different client.  See
         *  .xo-backlog/xo-websock/issues/08.
         *
         *  Holds only the bookkeeping; WebserverImpl does everything
         *  libwebsockets-related.  A template over the per-session record so
         *  it can be unit-tested with a fake one.
         *
         *  THREADING: every method may be called from any thread; one internal
         *  mutex.  with_session() and for_each() run their function WITH the
         *  mutex held, so that function must not re-enter this table.
         **/
        template <typename Recd>
        class WsSessionTable {
        public:
            using SessionId = std::uint64_t;

        public:
            /** a fresh id, never returned before by this table.  Starts at 1. **/
            SessionId next_id() {
                std::lock_guard<std::mutex> lock(this->mutex_);

                return this->next_id_++;
            }

            /** add @p recd as session @p id, at session open.
             *  require: @p id came from next_id(), and is not in the table
             **/
            void insert(SessionId id, std::unique_ptr<Recd> recd) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                assert(id < this->next_id_);

                [[maybe_unused]] bool inserted
                    = this->session_map_.emplace(id, std::move(recd)).second;

                assert(inserted);
            }

            /** remove session @p id and hand over its record, at session
             *  close; null if absent.  The caller disposes of the record with
             *  the mutex released.
             **/
            std::unique_ptr<Recd> take(SessionId id) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                auto ix = this->session_map_.find(id);

                if (ix == this->session_map_.end())
                    return nullptr;

                std::unique_ptr<Recd> retval = std::move(ix->second);
                this->session_map_.erase(ix);

                return retval;
            }

            /** run @p fn on session @p id's record, with the mutex held.
             *  Returns false, without calling @p fn, if @p id is not live --
             *  e.g. a session that has closed.
             **/
            template <typename Fn>
            bool with_session(SessionId id, Fn && fn) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                auto ix = this->session_map_.find(id);

                if (ix == this->session_map_.end())
                    return false;

                fn(*(ix->second));

                return true;
            }

            /** record for session @p id, or null; returned with the mutex
             *  RELEASED, for callers that must run code which re-enters the
             *  server.
             *
             *  Valid only while nothing can take() @p id concurrently: in the
             *  webserver, only its service thread calls take(), so the service
             *  thread may use this.
             **/
            Recd * find_owner_thread(SessionId id) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                auto ix = this->session_map_.find(id);

                return (ix == this->session_map_.end()) ? nullptr : ix->second.get();
            }

            /** run @p fn on every live session's record, mutex held **/
            template <typename Fn>
            void for_each(Fn && fn) {
                std::lock_guard<std::mutex> lock(this->mutex_);

                for (auto & ix : this->session_map_)
                    fn(*(ix.second));
            }

            /** as for_each above, read-only: @p fn gets Recd const & **/
            template <typename Fn>
            void for_each(Fn && fn) const {
                std::lock_guard<std::mutex> lock(this->mutex_);

                for (auto const & ix : this->session_map_)
                    fn(static_cast<Recd const &>(*(ix.second)));
            }

            /** as for_each const, in increasing id order -- the table is
             *  unordered, so this sorts, all under the mutex.
             *  @p fn gets (SessionId, Recd const &)
             **/
            template <typename Fn>
            void for_each_by_id(Fn && fn) const {
                std::lock_guard<std::mutex> lock(this->mutex_);

                std::vector<std::pair<SessionId, Recd const *>> v;
                v.reserve(this->session_map_.size());

                for (auto const & ix : this->session_map_)
                    v.emplace_back(ix.first, ix.second.get());

                std::sort(v.begin(), v.end(),
                          [](auto const & x, auto const & y) { return x.first < y.first; });

                for (auto const & ix : v)
                    fn(ix.first, *(ix.second));
            }

            /** number of live sessions **/
            std::size_t size() const {
                std::lock_guard<std::mutex> lock(this->mutex_);

                return this->session_map_.size();
            }

        private:
            /* guards everything below */
            mutable std::mutex mutex_;
            /* next id to hand out; only ever increases */
            SessionId next_id_ = 1;
            /* live sessions */
            std::unordered_map<SessionId, std::unique_ptr<Recd>> session_map_;
        }; /*WsSessionTable*/
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionTable.hpp */
