/** @file LockableTdx.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include <memory>

namespace xo {
    namespace reflect {
        /** @brief extra type-associated information for a lockable
         *  (std::mutex, std::shared_mutex): how a reader acquires and
         *  releases it.
         *
         *  Its metatype is mt_atomic, with no children.  What it adds is the
         *  lock operations a traversal needs to read members the lockable
         *  guards (StructMemberDecl::guarded_by, visit_members_guarded).  A
         *  traversal only reads, so the operations are a reader's: a
         *  std::shared_mutex is taken shared, a std::mutex exclusively.
         *
         *  Reached through TypeDescrExtra::lockable_info()
         *  (TypeDescr::is_lockable()).  Installed by EstablishTdx
         *  (Reflect.hpp).  See .xo-backlog/xo-reflect/issues/08.
         **/
        class LockableTdx : public TypeDescrExtra {
        public:
            /** acquire, or release, the lockable at @p lockable **/
            using LockFn = void (*)(void * lockable);
            /** acquire the lockable at @p lockable if free; true if acquired **/
            using TryLockFn = bool (*)(void * lockable);

        public:
            static std::unique_ptr<LockableTdx> make(LockFn read_lock,
                                                     LockFn read_unlock,
                                                     TryLockFn try_read_lock);

            /** acquire @p lockable for reading; blocks **/
            void read_lock(void * lockable) const { read_lock_(lockable); }
            /** release @p lockable, acquired by read_lock() or try_read_lock() **/
            void read_unlock(void * lockable) const { read_unlock_(lockable); }
            /** acquire @p lockable for reading if free; true if acquired.
             *  Undefined, as for the lockable itself, if this thread holds it
             **/
            bool try_read_lock(void * lockable) const { return try_read_lock_(lockable); }

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_atomic; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual uint32_t n_child_fixed() const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            virtual const TypeDescrBase * fixed_child_td(uint32_t i) const override;
            virtual std::string const & struct_member_name(uint32_t i) const override;
            virtual const LockableTdx * lockable_info() const override { return this; }

        private:
            LockableTdx(LockFn read_lock, LockFn read_unlock, TryLockFn try_read_lock)
                : read_lock_{read_lock}, read_unlock_{read_unlock},
                  try_read_lock_{try_read_lock} {}

        private:
            /** acquire for reading **/
            LockFn read_lock_ = nullptr;
            /** release **/
            LockFn read_unlock_ = nullptr;
            /** acquire for reading, if free **/
            TryLockFn try_read_lock_ = nullptr;
        }; /*LockableTdx*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end LockableTdx.hpp */
