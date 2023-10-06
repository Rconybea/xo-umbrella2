/* file TaggedRcptr.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "TaggedPtr.hpp"
// causes #include cycle, reflect/Reflect.hpp includes this header
//#include "reflect/Reflect.hpp"
#include "refcnt/Refcounted.hpp"

namespace xo {
  namespace reflect {
    /* Tagged reference-counted pointer.
     * Like TaggedPtr,  but also maintains reference count.
     *
     * note that refcounting behavior is lost if assigned to a TaggedPtr variable!
     */
    class TaggedRcptr : public TaggedPtr {
    public:
      using Refcount = ref::Refcount;

    public:
      TaggedRcptr(TypeDescr td, Refcount * x) : TaggedPtr(td, x) {
    ref::intrusive_ptr_add_ref(x);
      }
      TaggedRcptr(TaggedRcptr const & x) : TaggedPtr(x) {
    ref::intrusive_ptr_add_ref(x.rc_address());
      }
      TaggedRcptr(TaggedRcptr && x) : TaggedPtr(std::move(x)) {
    /* since we're moving from x,  need to make sure x.dtor
     * doesn't decrement refcount
     */
    x.assign_address(nullptr);
      }
      ~TaggedRcptr() {
    ref::intrusive_ptr_release(this->rc_address());
      }

      /* causes #include cycle,  see [reflect/Reflect.hpp] */
#ifdef NOT_IN_USE
      /* require: T --isa--> ref::Refcount */
      template<typename T>
      static TaggedRcptr make(T * x) { return TaggedRcptr(Reflect::require<T>(), x); }
#endif

      Refcount * rc_address() const {
    return reinterpret_cast<Refcount *>(this->address());
      } /*rc_address*/

      TaggedRcptr & operator=(TaggedRcptr const & rhs) {
    Refcount * x = rhs.rc_address();
    Refcount * old = this->rc_address();

    TaggedPtr::operator=(rhs);

    if (x != old) {
      intrusive_ptr_release(old);
      intrusive_ptr_add_ref(x);
    }

    return *this;
      } /*operator=*/

      TaggedRcptr & operator=(TaggedRcptr && rhs) {
    /* swap pointers + type descriptions;
     * then don't need to touch refcounts
     */
    std::swap(this->td_, rhs.td_);
    std::swap(this->address_, rhs.address_);

    return *this;
      } /*operator=*/

      void display(std::ostream & os) const;
      std::string display_string() const;
    }; /*TaggedRcptr*/

    inline std::ostream & operator<<(std::ostream & os, TaggedRcptr const & x) {
      x.display(os);
      return os;
    } /*operator<<*/

  } /*namespace reflect*/
} /*namespace xo*/

/* end TaggedRcptr.hpp */
