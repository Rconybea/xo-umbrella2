/** @file DArenaVector.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArena.hpp"
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

            bool empty() const { return size_ == 0; }
            size_type size() const { return size_; }
            size_type max_size() const { return capacity(); }
            size_type capacity() const { return store_.reserved() / sizeof(T); }

            T & operator[](size_t i) { return *(this->_address_of(i)); }
            const T & operator[](size_t i) const { return *(this->_address_of(i)); }

            T & at(size_type i) { _check_valid_index(i); return *(this->_address_of(i)); }
            const T & at(size_type i) const { _check_valid_index(i); return *(this->_address_of(i)); }

            iterator begin() noexcept { return this->_address_of(0); }
            iterator end() noexcept { return this->_address_of(size_); }
            const_iterator cbegin() const noexcept { return this->_address_of(0); }
            const_iterator begin() const noexcept { return this->cbegin(); }
            const_iterator cend() const noexcept { return this->_address_of(size_); }
            const_iterator end() const noexcept { return this->cend(); }

            constexpr T * data() { return store_.lo_; }
            constexpr const T * data() const { return store_.lo_; }

            void reserve(size_type z);
            void resize(size_type z);
            void shrink_to_fit();
            void clear();

            void push_back(T && x);
            void push_back(const T & x);

            void swap(DArenaVector & other) noexcept;

        private:
            T * _address_of(size_type i) { return *((T *)store_.lo_) + i; }
            const T * _address_of(size_type i) const { return *((const T *)store_.lo_) + i; }

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
        {}

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
                    // nothing to do
                    ;
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
        void
        DArenaVector<T>::push_back(T && x) {
            size_type z = size_ + 1;
            size_type req_z = z * sizeof(T);

            store_.expand(req_z);

            T * addr = this->address_of(size_);

            new (addr) T{std::move(x)};

            size_ = z;
        }

        template <typename T>
        void
        DArenaVector<T>::push_back(const T & x) {
            size_type z = size_;
            this->resize(z + 1);
            (*this)[z] = x;
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
