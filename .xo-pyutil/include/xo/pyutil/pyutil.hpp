/* @file pyutil.hpp
 *
 * utility stuff to be used across multiple pybind11 .cpp files
 */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "xo/refcnt/Unowned.hpp"
#include <pybind11/pybind11.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 *
 * Need declaration like this before any pybind11 bindings
 * that expose an object of types like
 * (a) intrusive_ptr<T> or
 * (b) T * / T const * / T & / T const & to python.
 * If this were not done,  pybind11 would by default use unique_ptr<intrusive_ptr<T>>
 * (ok but inefficient) or unique_ptr<T> (fatal!)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

/* xo::ref::unowned_ptr<T> is an unmanaged pointer.
 * use this for immortal objects that pybind11 must not delete.
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::unowned_ptr<T>, true);

/* end pyutil.hpp */
