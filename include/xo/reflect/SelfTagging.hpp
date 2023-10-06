/* file SelfTagging.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "Refcounted.hpp"
#include "TypeDescr.hpp"
#include "TaggedRcptr.hpp"

namespace xo {
  namespace reflect {
    /* a self-tagging object uses reflection to preserve type information
     * until runtime.   Can use the reflected information to traverse
     * object representation (e.g. for printing / serialization)
     * without repetitive/bulky boilerplate.
     *
     * For pybind11 need to have concrete (non-template) apis,
     * helpful to have various classes inherit SelfTagging
     *
     * For example see [printjson/PrintJson.hpp]
     */
    class SelfTagging : public ref::Refcount {
    public:
      virtual TaggedRcptr self_tp() = 0;
    }; /*SelfTagging*/
  } /*namespace reflect*/
} /*namespace xo*/

/* end SelfTagging.hpp */
