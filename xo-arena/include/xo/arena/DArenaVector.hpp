/** @file DArenaVector.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArena.hpp"
#include <xo/reflectutil/typeseq.hpp>
#include <cstring> // for ::memset()
#include <stdexcept>

namespace xo {
    namespace mm {
        /** @brief vector of T using dedicated DArena for storage
         *
         *  Replicate (to the extent feasible) std::vector<T>
         *  behavior, but using a dedicated DArena to provide storage
         *
         *  Unlike std::vector:
         *  1. does not support copying
         *  2. capacity fixed at construction time
         *
         *  @tparam T element type. Must be Erasable
         **/
        template <typename T>
        struct DArenaVector {
        public:
            using value_type = T;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using reference = value_type &;
            using const_reference = const value_type &;
            using iterator = value_type *;
            using const_iterator = const value_type *;
            using Checkpoint = DArena::Checkpoint;

        public:
            /** null ctor **/
            DArenaVector() = default;
            /** create arena-backed vector from @p cfg.  Will reserve memory for allocation.
             *  @p cfg size is desired size for *vector* storage,
             *  before accounting for arena overhead.
             **/
            DArenaVector(const ArenaConfig & cfg);
            /** ctor from already-mapped (but not committed) address range_type
             *  vector has size zero
             **/
            DArenaVector(const ArenaConfig & cfg,
                         size_type page_z,
                         size_type arena_align_z,
                         DArena::value_type lo,
                         DArena::value_type hi);
            /** not intended to be copyable **/
            DArenaVector(const DArenaVector &) = delete;
            /** move ctor **/
            DArenaVector(DArenaVector && other);
            /** releases mapped memory **/
            ~DArenaVector();

            /** create empty vector using @p cfg to configure backing store.
             *  @p cfg size is desired size for *vector* storage,
             *  before accounting for arena overhead.
             **/
            static DArenaVector map(const ArenaConfig & cfg);

            /** true iff vector is emtpy **/
            bool empty() const { return size_ == 0; }
            size_type size() const { return size_; }
            size_type max_size() const { return capacity(); }
            size_type capacity() const {
                return (store_.reserved()
                        - store_.preamble_z()
                        - store_.alloc_header_z()
                        - store_.padded_guard_z())
                       / sizeof(T);
            }
            /** allocation overhead for this vector:
             *  arena memory used for metadata, not available for element storage
             **/
            size_type overhead_z() const { return store_.preamble_z() + store_.per_alloc_overhead_z(); }

            /** get reference to element at zero-based index @p i. Do not check bounds **/
            T & operator[](size_t i) noexcept { return *(this->_address_of(i)); }
            const T & operator[](size_t i) const noexcept { return *(this->_address_of(i)); }

            /** get reference to element at zero-based index @p i. Do check bounds **/
            T & at(size_type i) { _check_valid_index(i); return *(this->_address_of(i)); }
            const T & at(size_type i) const { _check_valid_index(i); return *(this->_address_of(i)); }

            /** get to at first element of vector. Same as @p end if vector is empty **/
            iterator begin() noexcept { return this->_address_of(0); }
            /** get iterator to end of vector - "one past the last element" **/
            iterator end() noexcept { return this->_address_of(size_); }
            const_iterator cbegin() const noexcept { return this->_address_of(0); }
            const_iterator begin() const noexcept { return this->cbegin(); }
            const_iterator cend() const noexcept { return this->_address_of(size_); }
            const_iterator end() const noexcept { return this->cend(); }

            T & back() { return *(this->_address_of(size_ - 1)); }
            const T & back() const { return *(this->_address_of(size_ - 1)); }

            constexpr const DArena * store() const { return &store_; }
            constexpr T * data() { return base_; /*return reinterpret_cast<T*>(store_.lo_);*/ }
            constexpr const T * data() const { return base_; /*reinterpret_cast<const T*>(store_.lo_);*/ }

            /** arena used for element storage
             *  (Might prefer obj<AResourceVisitor> here; refrain to avoid leveling violation)
             **/
            void visit_pools(const MemorySizeVisitor & fn) const { store_.visit_pools(fn); }

            /** reserve space, if possible, for at least @p z elements.
             *  Always limited by ArenaConfig.size_
             **/
            bool reserve(size_type z);
            /** resize to size @p z.  Return true on success. May fail iff oom. **/
            bool resize(size_type z);
            void shrink_to_fit();
            /** reset vector to empty state **/
            void clear();

            T & insert(size_type pos, T && x);
            T & insert(size_type pos, const T & x);

            void erase(size_type pos);

            T * push_back(T && x);
            T * push_back(const T & x);

            void pop_back();

            void swap(DArenaVector & other) noexcept;

            DArenaVector & operator=(DArenaVector && x) noexcept;

        private:
            /** total arena memory needed to store @p n elements of type T **/
            size_type _memory_z(size_type n) const {
                size_type req_z = n * sizeof(T);

                return (store_.preamble_z()
                        + store_.alloc_header_z()
                        + req_z
                        + padding::alloc_padding(req_z)
                        + store_.padded_guard_z()
                        );
            }

            T * _address_of(size_type i) { return base_ + i; /*((T *)store_.lo_) + i;*/ }
            const T * _address_of(size_type i) const { return base_ + i; /* ((const T *)store_.lo_) + i;*/ }

            void _check_valid_index(size_type i) const;

            /** ensure arena alloc for this vector matches size @p z,
             *  given @p z does not shrink size.
             *  works for initial commit+alloc
             **/
            bool _increase_alloc_aux(size_type z);

            /** ensure arena alloc for this vector matches size @p z,
             *  given @p z shrinks size.
             **/
            void _decrease_alloc_aux(size_type z);

        private:
            size_type size_ = 0;

            /** arena state before first alloc **/
            Checkpoint prealloc_;
            /** address of first vector element **/
            T * base_ = nullptr;

            DArena store_;
        };

        template <typename T>
        DArenaVector<T>::DArenaVector(const ArenaConfig & cfg)
        {
            *this = map(cfg);
        }

        template <typename T>
        DArenaVector<T>::DArenaVector(const ArenaConfig & cfg,
                                      size_type page_z,
                                      size_type arena_align_z,
                                      DArena::value_type lo,
                                      DArena::value_type hi)
        : base_{nullptr},
          store_{cfg, page_z, arena_align_z, lo, hi}
        {
        }

        template <typename T>
        DArenaVector<T>::DArenaVector(DArenaVector && other)
        : size_{other.size_}, prealloc_{other.prealloc_}, base_{other.base_}, store_{std::move(other.store_)}
        {
            other.prealloc_ = Checkpoint();
            other.base_ = nullptr;
            other.size_ = 0;
        }

        template <typename T>
        DArenaVector<T>::~DArenaVector()
        {
            if constexpr (std::is_trivially_destructible_v<T>) {
                // nothing to do
            } else {
                // invoke destructor for each element
                for (size_type i = 0, n = size(); i < n; ++i) {
                    T & x = (*this)[i];

                    x.~T();
                }
            }
        }

        template <typename T>
        DArenaVector<T> &
        DArenaVector<T>::operator=(DArenaVector && other) noexcept
        {
            this->size_ = other.size_;
            this->prealloc_ = other.prealloc_;
            this->base_ = other.base_;
            this->store_ = std::move(other.store_);

            other.size_ = 0;
            other.prealloc_ = Checkpoint();
            other.base_ = nullptr;

            return *this;
        }

        template <typename T>
        DArenaVector<T>
        DArenaVector<T>::map(const ArenaConfig & cfg)
        {
            DArenaVector<T> retval;

            size_type element_z = cfg.size();

            ArenaConfig arena_cfg
                = cfg.with_size(cfg.preamble_z()
                                + cfg.alloc_header_z()
                                + element_z
                                + cfg.padded_guard_z());

            retval.store_ = DArena::map(arena_cfg);
            /* memory not committed yet */
            retval.base_ = nullptr;

            return retval;
        }

        template <typename T>
        bool
        DArenaVector<T>::reserve(size_type z) {
            size_t mem_z = this->_memory_z(z);

            return store_.expand(mem_z,
                                 __PRETTY_FUNCTION__);
        }

        template <typename T>
        bool
        DArenaVector<T>::_increase_alloc_aux(size_type z) {
            // technically includes the equals-current-size case when size_ is zero

            using xo::reflect::typeseq;

            // expand arena to accomodate

            size_type mem_z = this->_memory_z(z);

            if (!store_.expand(mem_z, __PRETTY_FUNCTION__))
                return false;

            if (!base_) [[unlikely]] {
                prealloc_ = store_.checkpoint();

                // first alloc for this ArenaVector
                base_ = reinterpret_cast<T *>(store_.alloc(typeseq::id<T[]>(), z * sizeof(T)));

                assert(base_); // success guaranteed by preceding expand() call
            } else {
                store_.restore(prealloc_);

                // alloc to get alloc header + guard bytes
                auto b = reinterpret_cast<T *>(store_.alloc(typeseq::id<T[]>(), z * sizeof(T)));

                if (b != base_)
                    assert(false);
            }

            return true;
        }

        template <typename T>
        void
        DArenaVector<T>::_decrease_alloc_aux(size_type z) {
            using xo::reflect::typeseq;

            assert(base_);

            store_.restore(prealloc_);

            if (z > 0) {
                auto b = reinterpret_cast<T *>(store_.alloc(typeseq::id<T[]>(), z * sizeof(T)));

                if(b != base_)
                    assert(false);
            }
        }

        template <typename T>
        bool
        DArenaVector<T>::resize(size_type z) {
            if (z >= size_) {
                // expand arena to accomodate

                bool ok = this->_increase_alloc_aux(z);

                if (!ok) {
                    return false;
                }

                // run ctors
                if constexpr (std::is_trivially_constructible_v<T>) {
                    // trivially constructible -> init new element memory to zeroes
                    ::memset(this->_address_of(size_), 0, (z - size_) * sizeof(T));
                } else {
                    for (size_type i = size_; i < z; ++i) {
                        void * addr = &(*this)[i];

                        new (addr) T();
                    }
                }
            } else {
                assert(base_);

                if constexpr (std::is_trivially_destructible_v<T>) {
                    // nothing to do
                } else {
                    // invoke destructor for each discarded element
                    for (size_type i = z; i < size_; ++i) {
                        T & x = (*this)[i];

                        x.~T();
                    }
                }

                this->_decrease_alloc_aux(z);
            }

            this->size_ = z;

            return true;
        }

        template <typename T>
        void
        DArenaVector<T>::shrink_to_fit() {
            // could in principle release unused mapped pages here
        }

        template <typename T>
        void
        DArenaVector<T>::clear() {
            this->resize(0);
        }

        template <typename T>
        void
        DArenaVector<T>::_check_valid_index(size_type i) const {
            if (size_ <= i)
                throw std::out_of_range("DArenaVector index out of bounds");
        }

        template <typename T>
        T &
        DArenaVector<T>::insert(size_type pos, T && x) {
            size_type z = size_ + 1;

            if (this->_increase_alloc_aux(z)) {
                // move elements [i .. z-1] right by one position.
                // must proceed in reverse order!
                for (size_type ip1 = size_; ip1 > pos; --ip1) {
                    (*this)[ip1] = std::move((*this)[ip1-1]);
                }

                T * addr = this->_address_of(pos);

                new (addr) T{std::move(x)};

                this->size_ = size_ + 1;

                return *addr;
            } else {
                assert(false);

                T * x = nullptr;

                return *x;
            }
        }

        template <typename T>
        T &
        DArenaVector<T>::insert(size_type pos, const T & x) {
            size_type z = size_ + 1;

            if (this->_increase_alloc_aux(z)) {
                // move elements [i .. z-1] right by one position.
                // must proceed in reverse order!
                for (size_type ip1 = size_; ip1 > pos; --ip1) {
                    (*this)[ip1] = std::move((*this)[ip1-1]);
                }

                T * addr = this->_address_of(pos);

                new (addr) T{x};

                this->size_ = size_ + 1;

                return *addr;
            } else {
                assert(false);

                T * x = nullptr;

                return *x;
            }
        }

        template <typename T>
        void
        DArenaVector<T>::erase(size_type pos) {
            // move elements [pos+1 .. z-1] left by one position.

            if (pos >= size_) [[unlikely]]
                return;

            for (size_type i = pos; i+1 < size_; ++i) {
                (*this)[i] = std::move((*this)[i+1]);
            }

            --(this->size_);

            this->_decrease_alloc_aux(size_);
        }

        template <typename T>
        T *
        DArenaVector<T>::push_back(T && x) {
            size_type z = size_ + 1;

            if (!this->_increase_alloc_aux(z)) [[unlikely]] {
                return nullptr;
            }

            T * addr = this->_address_of(size_);

            new (addr) T{std::move(x)};

            this->size_ = z;

            return addr;
        }

        template <typename T>
        T *
        DArenaVector<T>::push_back(const T & x) {
            size_type z = size_ + 1;

            if (!this->_increase_alloc_aux(z)) [[unlikely]] {
                return nullptr;
            }

            T * addr = this->_address_of(size_);

            new (addr) T{x};

            this->size_ = z;

            return addr;
        }

        template <typename T>
        void
        DArenaVector<T>::pop_back() {
            if (size_ > 0) [[likely]] {
                --size_;

                if constexpr (std::is_trivially_destructible_v<T>) {
                    // nothing to do
                } else {
                    T & x = (*this)[size_];

                    x.~T();
                }

                this->_decrease_alloc_aux(size_);
            }
        }

        template <typename T>
        void
        DArenaVector<T>::swap(DArenaVector & other) noexcept {
            std::swap(size_, other.size_);
            std::swap(prealloc_, other.prealloc_);
            std::swap(base_, other.base_);
            std::swap(store_, other.store_);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DArenaVector.hpp */
