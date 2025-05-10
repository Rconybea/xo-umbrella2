/* @file Refcounted.hpp */

#pragma once

#include "xo/indentlog/scope.hpp"
#include "xo/cxxutil/demangle.hpp"

//#include <boost/intrusive_ptr.hpp>
#include <atomic>
#include <cassert>

namespace xo {
    namespace ref {
        template <typename T>
        class intrusive_ptr;
    }

    template <typename T>
    using rp = ref::intrusive_ptr<T>;

    namespace ref {
        class Refcount;

        template<typename T>
        class Borrow;

        /* originally used boost::instrusive_ptr<>.
         * ran into a bug.  probably mine,  but implemented
         * refcounting inline for debugging
         */
        template<typename T>
        class intrusive_ptr {
        public:
            using element_type = T;

        public:
            intrusive_ptr() : ptr_(nullptr) {}
            intrusive_ptr(T * x) : ptr_(x) {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_ctor(sc_self_type, this, x);
#endif
                intrusive_ptr_add_ref(ptr_);
            } /*ctor*/

            /* NOTE: need exactly this form for copy-constructor
             *       clang11 will not recognize template form below as
             *       supplying copy ctor,   and default version is broken for
             *       instrusive_ptr.
             */
            intrusive_ptr(intrusive_ptr const & x) : ptr_(x.get()) {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_cctor(sc_self_type, this, x.get());
#endif
                intrusive_ptr_add_ref(ptr_);
            } /*cctor*/

            /* create from instrusive pointer to some related type S */
            template<typename S>
            intrusive_ptr(intrusive_ptr<S> const & x) : ptr_(x.get()) {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_cctor(sc_self_type, this, x.get());
#endif
                intrusive_ptr_add_ref(ptr_);
            } /*cctor*/

            /* move ctor -- in this case don't need to update refcount */
            intrusive_ptr(intrusive_ptr && x) : ptr_{std::move(x.ptr_)} {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_mctor(sc_self_type, this, ptr_);
#endif
                /* since we're moving from x,  need to make sure x dtor
                 * doesn't decrement refcount
                 */
                x.ptr_ = nullptr;
            }

            /* aliasing ctor.  see ctor (8) here:
             *   [[https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr]]
             * and this dicsussion:
             *   [[https://stackoverflow.com/questions/49178231/pybind11-multiple-inheritance-with-custom-holder-type-fails-to-cast-to-base-type/73131206#73131206]]
             */
            template<typename Y>
            intrusive_ptr(intrusive_ptr<Y> const & /*x*/, element_type * y) : ptr_{y} {
                if (std::is_same<Y, element_type>()) {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                    intrusive_ptr_log_actor(sc_self_type, this, y);
#endif
                    intrusive_ptr_add_ref(ptr_);
                    ; /* trivial aliasing,  proceed */
                } else {
                    using xo::xtag;
                    throw std::runtime_error(tostr("attempt to use aliasing ctor with",
                                                   xtag("Y", reflect::type_name<Y>()),
                                                   xtag("T", reflect::type_name<T>())));
                }
            } /*ctor*/

            ~intrusive_ptr() {
                T * x = this->ptr_;

#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_dtor(sc_self_type, this, x);
#endif

                this->ptr_ = nullptr;

                intrusive_ptr_release(x);
            } /*dtor*/

            static bool compare(intrusive_ptr<T> const & x,
                                intrusive_ptr<T> const & y) {
                return ptrdiff_t(x.get() - y.get());
            }

            Borrow<T> borrow() const;

            T * get() const { return ptr_; }

            T * operator->() const { return ptr_; }

            operator bool() const { return ptr_ != nullptr; }

            intrusive_ptr<T> & operator=(intrusive_ptr<T> const & rhs) {
                T * x = rhs.get();

#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_assign(sc_self_type, this, x);
#endif

                T * old = this->ptr_;
                this->ptr_ = x;

                intrusive_ptr_add_ref(x);
                intrusive_ptr_release(old);

                return *this;
            } /*operator=*/

            intrusive_ptr<T> & operator=(intrusive_ptr<T> && rhs) {
#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
                intrusive_ptr_log_massign(sc_self_type, this, rhs.get());
#endif

                std::swap(this->ptr_, rhs.ptr_);

                /* dtor on rhs will decrement refcount on old value of this->ptr_
                 * don't increment for new value,  since refcount just transfers from rhs to *this
                 */

                return *this;
            } /*operator=*/

        private:
            static constexpr std::string_view sc_self_type = xo::reflect::type_name<intrusive_ptr<T>>();

        private:
            T * ptr_ = nullptr;
        }; /*intrusive_ptr*/

        template<typename T>
        inline bool operator==(intrusive_ptr<T> const & x,
                               intrusive_ptr<T> const & y) { return intrusive_ptr<T>::compare(x, y) == 0; }

        class Refcount {
        public:
            Refcount() : reference_counter_(0) {}
            /* WARNING: virtual dtor here is essential,
             *          since it's what allows us to invoke delete on a Refcount*,
             * for an object of some derived class type T.  Otherwise clang
             * will use different addresses for Refcount-part and T-part of
             * such instance,  which means pointer given to delete will not be
             * the same as pointer returned from new
             */
            virtual ~Refcount() = default;

            uint32_t reference_counter() const { return reference_counter_.load(); }

        private:
            friend uint32_t intrusive_ptr_refcount(Refcount *);
            friend void intrusive_ptr_add_ref(Refcount *);
            friend void intrusive_ptr_release(Refcount *);

        private:
            std::atomic<uint32_t> reference_counter_;
        }; /*Refcount*/

        inline uint32_t
        intrusive_ptr_refcount(Refcount * x) {
            /* reporting accurately for diagnostics */
            if (x)
                return x->reference_counter_.load();
            else
                return 0;
        } /*intrusive_ptr_refcount*/

#ifdef XO_INTRUSIVE_PTR_ENABLE_LOGGING
        extern void intrusive_ptr_set_debug(bool x);
        extern void intrusive_ptr_log_ctor(std::string_view const & self_type,
                                           void * this_ptr,
                                           Refcount * x);
        /* here actor short for 'aliasing ctor' */
        extern void intrusive_ptr_log_actor(std::string_view const & self_type,
                                            void * this_ptr,
                                            Refcount * x);
        extern void intrusive_ptr_log_cctor(std::string_view const & self_type,
                                            void * this_ptr,
                                            Refcount * x);
        extern void intrusive_ptr_log_mctor(std::string_view const & self_type,
                                            void *this_ptr,
                                            Refcount * x);
        extern void intrusive_ptr_log_dtor(std::string_view const & self_type,
                                           void * this_ptr,
                                           Refcount * x);
        extern void intrusive_ptr_log_assign(std::string_view const & self_type,
                                             void * this_ptr,
                                             Refcount * x);
        extern void intrusive_ptr_log_massign(std::string_view const & self_type,
                                              void *this_ptr,
                                              Refcount * x);
#endif
        extern void intrusive_ptr_add_ref(Refcount * x);
        extern void intrusive_ptr_release(Refcount * x);

        template<typename T>
        inline std::ostream &
        operator<<(std::ostream & os, intrusive_ptr<T> const & x) {
            if(x.get()) {
                os << *(x.get());
            } else {
                os << "<nullptr " << reflect::type_name<T>() << ">";
            }
            return os;
        } /*operator<<*/

        /** Wrap a (presumably non-reference-counted) class T so that it has a refcount.
         **/
        template <typename T>
        class RefcountWrapper : public Refcount, public T {
        public:
            template<typename... Args>
            RefcountWrapper(Args... args) : Refcount(), T(std::forward<Args...>(args...)) {}
        };

        /* borrow a reference-counted pointer to pass down the stack
         * 1. borrowed pointer intended to replace:
         *     a. code like
         *          foo(rp<T> x),
         *        passing rp<T> by value requires increment/decrement pair,
         *        which is superfluous given that caller holds reference throughout
         *     b. code like
         *          foo(rp<T> const & x)
         *        passing rp<T> by reference requires double-indirection in called
         *        code
         * 2. borrowed pointer does not check/maintain reference count.
         *    it should never be stored in a struct;  intended strictly
         *    to be passed down stack
         * 3. just the same, want to be able to copy the borrowed pointer,
         *    to avoid double-indirection
         * 4. also can promote borrowed pointer to full reference-counted
         *    whenever desired
         */
        template<typename T>
        class Borrow {
        public:
            Borrow() = default;

            template <typename S>
            Borrow(rp<S> const & x) : ptr_(x.get()) {}

            template <typename S>
            Borrow(S * x) : ptr_(x) {}

            Borrow(Borrow const & x) = default;

            /* convert from another borrow,   if it has compatible pointer type */
            template<typename S>
            Borrow(Borrow<S> const & x) : ptr_(x.get()) {}

            /* dynamic cast from a pointer to an object of some convertible type */
            template<typename S>
            static Borrow<T> from(Borrow<S> x) {
                return Borrow(dynamic_cast<T *>(x.get()));
            } /*from*/

            /* promote from native pointer */
            static Borrow<T> from_native(T * x) {
                return Borrow(x);
            } /*from_native*/

            T * get() const { return ptr_; }

            rp<T> promote() const { return rp<T>(ptr_); }

            T & operator*() const { return *ptr_; }
            T * operator->() const { return ptr_; }

            operator bool() const { return ptr_ != nullptr; }

            static int32_t compare(Borrow const & x, Borrow const & y) {
                return ptrdiff_t(x.get() - y.get());
            } /*compare*/

            static int32_t compare(rp<T> const & x, Borrow const & y) {
                return ptrdiff_t(x.get() - y.get());
            } /*compare*/

            template <typename S>
            Borrow<T> & operator=(const Borrow<S> & x) {
                ptr_ = x.get();
                return *this;
            }

            Borrow& operator=(const Borrow& x) {
                ptr_ = x.get();
                return *this;
            }

        private:
            T * ptr_ = nullptr;
        }; /*Borrow*/

        template<typename T>
        inline bool operator==(Borrow<T> x, Borrow<T> y) { return Borrow<T>::compare(x, y) == 0; }

        template<typename T>
        inline bool operator==(rp<T> const & x, Borrow<T> y) { return Borrow<T>::compare(x, y) == 0; }

        template<typename T>
        using brw = Borrow<T>;

        template<typename T>
        Borrow<T>
        intrusive_ptr<T>::borrow() const {
            return Borrow<T>(*this);
        } /*borrow*/

        template<typename T>
        inline std::ostream &
        operator<<(std::ostream & os, Borrow<T> x) {
            if (x) {
                os << *x;
            } else {
                os << "<nullptr "  << reflect::type_name<T>() << ">";
            }
            return os;
        } /*operator<<*/

    } /*namespace ref*/
} /*namespace xo*/

/* end Refcounted.hpp */
