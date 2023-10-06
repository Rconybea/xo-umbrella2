/* @file Unowned.hpp */

namespace xo {
    namespace ref {
        /* use this is a holder type for pointers that pybind11 should treat
         * as "not-my-problem".   in particular that pybind11 should never delete.
         */
        template<typename T>
        class unowned_ptr {
        public:
            unowned_ptr(T * x) : ptr_{x} {}
            unowned_ptr(unowned_ptr const & x) = default;
            ~unowned_ptr() = default;

            T * get() const { return ptr_; }
            T * operator->() const { return ptr_; }

            operator bool() const { return ptr_ != nullptr; }

            unowned_ptr<T> & operator=(unowned_ptr<T> const & rhs) = default;

        private:
            T * ptr_ = nullptr;
        }; /*unowned_ptr*/
    } /*namespace ref*/
} /*namespace xo*/

/* end Unowned.hpp */
