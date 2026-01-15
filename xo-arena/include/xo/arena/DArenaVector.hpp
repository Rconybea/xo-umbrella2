/** @file DArenaVector.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArena.hpp"
#include <stdexcept>
#include <cstring>  // for ::memset()

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

            /** null ctor **/
            DArenaVector() = default;
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

            /** create empty vector using @p cfg to configure backing store **/
            static DArenaVector map(const ArenaConfig & cfg);

            /** true iff vector is emtpy **/
            bool empty() const { return size_ == 0; }
            size_type size() const { return size_; }
            size_type max_size() const { return capacity(); }
            size_type capacity() const { return store_.reserved() / sizeof(T); }

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

            constexpr T * data() { return reinterpret_cast<T*>(store_.lo_); }
            constexpr const T * data() const { return reinterpret_cast<const T*>(store_.lo_); }

            /** reserve space, if possible, for at least @p z elements.
             *  Always limited by ArenaConfig.size_
             **/
            void reserve(size_type z);
            void resize(size_type z);
            void shrink_to_fit();
            /** reset vector to empty state **/
            void clear();

            T & insert(size_type pos, T && x);
            T & insert(size_type pos, const T & x);

            void erase(size_type pos);

            void push_back(T && x);
            void push_back(const T & x);

            void swap(DArenaVector & other) noexcept;

            DArenaVector & operator=(DArenaVector && x) noexcept;

        private:
            T * _address_of(size_type i) { return ((T *)store_.lo_) + i; }
            const T * _address_of(size_type i) const { return ((const T *)store_.lo_) + i; }

            void _check_valid_index(size_type i) const;

        private:
            size_type size_ = 0;
            DArena store_;
        };

        template <typename T>
        DArenaVector<T>::DArenaVector(const ArenaConfig & cfg,
                                      size_type page_z,
                                      size_type arena_align_z,
                                      DArena::value_type lo,
                                      DArena::value_type hi)
                : store_{cfg, page_z, arena_align_z, lo, hi}
        {}

        template <typename T>
        DArenaVector<T>::DArenaVector(DArenaVector && other)
                : size_{other.size_}, store_{std::move(other.store_)}
        {
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
            this->store_ = std::move(other.store_);

            other.size_ = 0;

            return *this;
        }

        template <typename T>
        DArenaVector<T>
        DArenaVector<T>::map(const ArenaConfig & cfg)
        {
            DArenaVector<T> retval;

            retval.store_ = std::move(DArena::map(cfg));

            return retval;
        }

        template <typename T>
        void
        DArenaVector<T>::reserve(size_type z) {
            store_.expand(z * sizeof(T));
        }

        template <typename T>
        void
        DArenaVector<T>::resize(size_type z) {
            if (z > size_) {
                // expand arena to accomodate
                size_t req_z = z * sizeof(T);

                store_.expand(req_z);

                // run ctors
                if constexpr (std::is_trivially_constructible_v<T>) {
                    ::memset(this->_address_of(size_), 0, req_z - (size_ * sizeof(T)));
                } else {
                    for (size_type i = size_; i < z; ++i) {
                        void * addr = &(*this)[i];

                        new (addr) T();
                    }
                }
            } else {
                if constexpr (std::is_trivially_destructible_v<T>) {
                    // nothing to do
                } else {
                    // invoke destructor for each discarded element
                    for (size_type i = z; i < size_; ++i) {
                        T & x = (*this)[i];

                        x.~T();
                    }
                }
            }

            this->size_ = z;
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
            {
                size_type new_z = size_ + 1;
                size_type req_z = new_z * sizeof(T);

                store_.expand(req_z);
            }

            // move elements [i .. z-1] right by one position.
            // must proceed in reverse order!
            for (size_type ip1 = size_; ip1 > pos; --ip1) {
                (*this)[ip1] = std::move((*this)[ip1-1]);
            }

            T * addr = this->_address_of(pos);

            new (addr) T{std::move(x)};

            this->size_ = size_ + 1;

            return *addr;
        }

        template <typename T>
        T &
        DArenaVector<T>::insert(size_type pos, const T & x) {
            {
                size_type new_z = size_ + 1;
                size_type req_z = new_z * sizeof(T);

                store_.expand(req_z);
            }

            // move elements [i .. z-1] right by one position.
            // must proceed in reverse order!
            for (size_type ip1 = size_; ip1 > pos; --ip1) {
                (*this)[ip1] = std::move((*this)[ip1-1]);
            }

            T * addr = this->_address_of(pos);

            new (addr) T{x};

            this->size_ = size_ + 1;

            return *addr;
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
        }

        template <typename T>
        void
        DArenaVector<T>::push_back(T && x) {
            size_type z = size_ + 1;
            size_type req_z = z * sizeof(T);

            if (this->store_.expand(req_z)) {
                T * addr = this->_address_of(size_);

                new (addr) T{std::move(x)};

                this->size_ = z;
            }
        }

        template <typename T>
        void
        DArenaVector<T>::push_back(const T & x) {
            size_type z = size_ + 1;

            if (this->store_.expand(z * sizeof(T))) {
                T * addr = this->_address_of(size_);

                new (addr) T{x};

                this->size_ = z;
            }
        }

        template <typename T>
        void
        DArenaVector<T>::swap(DArenaVector & other) noexcept {
            std::swap(size_, other.size_);
            std::swap(store_, other.store_);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DArenaVector.hpp */
