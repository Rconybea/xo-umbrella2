/** @file GuardedVisit.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "StructTdx.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/reflect/lockable/LockableTdx.hpp"
#include <cassert>
#include <cstdint>

namespace xo {
    namespace reflect {
        /** how visit_members_guarded acquires a guard **/
        enum class GuardMode {
            /** wait for it **/
            blocking,
            /** take it only if free; else report its members unreadable **/
            try_lock,
        };

        namespace detail {
            /* releases a guard visit_members_guarded acquired, on scope exit */
            class GuardRelease {
            public:
                GuardRelease(LockableTdx const * lockable, void * address, bool held)
                    : lockable_{lockable}, address_{address}, held_{held} {}
                GuardRelease(GuardRelease const &) = delete;
                GuardRelease & operator=(GuardRelease const &) = delete;
                ~GuardRelease() {
                    if (held_)
                        lockable_->read_unlock(address_);
                }

            private:
                LockableTdx const * lockable_ = nullptr;
                void * address_ = nullptr;
                bool held_ = false;
            };
        } /*namespace detail*/

        /** @brief visit the members of the reflected struct at @p object,
         *  holding each member's guard (StructMemberDecl::guarded_by) while
         *  visiting it.
         *
         *  Calls @p fn(uint32_t member_ix, bool readable): first for each
         *  unguarded member, then -- for each guard in turn -- for that
         *  guard's members, with the guard held.  Guards of one struct never
         *  nest; a guard is held across its whole group, so the group is one
         *  consistent snapshot, and @p fn may descend into a member (taking
         *  that member's own guards) while it is held.
         *
         *  @p readable is false only in GuardMode::try_lock, for the members
         *  of a guard another thread held; @p fn must not read them then.
         *
         *  The calling thread must hold none of these guards: a std::mutex
         *  is not recursive.  Releases what it holds if @p fn throws.
         *
         *  See .xo-backlog/xo-reflect/issues/08.
         **/
        template <typename Fn>
        void
        visit_members_guarded(TypeDescr struct_td, void * object, GuardMode mode, Fn && fn)
        {
            assert(struct_td->is_struct());

            StructTdx const * tdx = static_cast<StructTdx const *>(struct_td->tdextra());

            for (uint32_t i : tdx->unguarded_members())
                fn(i, true);

            for (uint32_t g = 0, n = tdx->n_guard(); g < n; ++g) {
                TaggedPtr guard = tdx->guard_tp(g, object);
                LockableTdx const * lockable = guard.td()->lockable_info();

                assert(lockable);

                bool held = false;

                if (mode == GuardMode::blocking) {
                    lockable->read_lock(guard.address());
                    held = true;
                } else {
                    held = lockable->try_read_lock(guard.address());
                }

                detail::GuardRelease release(lockable, guard.address(), held);

                for (uint32_t i : tdx->guard_members(g))
                    fn(i, held);
            }
        } /*visit_members_guarded*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end GuardedVisit.hpp */
