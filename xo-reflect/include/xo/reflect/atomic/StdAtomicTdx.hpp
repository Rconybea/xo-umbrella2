/** @file StdAtomicTdx.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include <cstddef>
#include <memory>

namespace xo {
    namespace reflect {
        /** @brief extra type-associated information for a std::atomic<T>:
         *  how to load its value, and the value's type.
         *
         *  A std::atomic<T> cannot be traversed in place -- its storage is
         *  not a T to read through, only load() is -- so it has no children,
         *  and its metatype is mt_atomic.  What it adds is a load: std::atomic
         *  requires T to be trivially copyable, so the current value can be
         *  copied into any suitably sized and aligned buffer, and reflected
         *  there as a T (value_td()).
         *
         *  Reached through TypeDescrExtra::std_atomic_info()
         *  (TypeDescr::is_std_atomic()).  Installed for every std::atomic<T>
         *  by EstablishTdx (Reflect.hpp).  See .xo-backlog/xo-reflect/issues/04.
         **/
        class StdAtomicTdx : public TypeDescrExtra {
        public:
            /** copy the value of the std::atomic<T> at @p atomic into @p dst:
             *  value_size() bytes, aligned to value_align()
             **/
            using LoadFn = void (*)(void const * atomic, void * dst);

        public:
            static std::unique_ptr<StdAtomicTdx> make(const TypeDescrBase * value_td,
                                                      std::size_t value_size,
                                                      std::size_t value_align,
                                                      LoadFn load);

            /** T, for a std::atomic<T> **/
            const TypeDescrBase * value_td() const { return value_td_; }
            /** sizeof(T) **/
            std::size_t value_size() const { return value_size_; }
            /** alignof(T) **/
            std::size_t value_align() const { return value_align_; }

            /** copy the current value of the std::atomic<T> at @p atomic into
             *  @p dst (std::atomic<T>::load()).  @p dst holds value_size()
             *  bytes, aligned to value_align()
             **/
            void load(void const * atomic, void * dst) const { load_(atomic, dst); }

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_atomic; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual uint32_t n_child_fixed() const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            virtual const TypeDescrBase * fixed_child_td(uint32_t i) const override;
            virtual std::string const & struct_member_name(uint32_t i) const override;
            virtual const StdAtomicTdx * std_atomic_info() const override { return this; }

        private:
            StdAtomicTdx(const TypeDescrBase * value_td,
                         std::size_t value_size,
                         std::size_t value_align,
                         LoadFn load)
                : value_td_{value_td}, value_size_{value_size},
                  value_align_{value_align}, load_{load} {}

        private:
            /** T **/
            const TypeDescrBase * value_td_ = nullptr;
            /** sizeof(T) **/
            std::size_t value_size_ = 0;
            /** alignof(T) **/
            std::size_t value_align_ = 0;
            /** std::atomic<T>::load() into a buffer **/
            LoadFn load_ = nullptr;
        }; /*StdAtomicTdx*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end StdAtomicTdx.hpp */
