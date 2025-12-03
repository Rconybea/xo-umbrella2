/** @file Object.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/reflect/SelfTagging.hpp"
//#include <cstdint>

namespace xo {
    namespace reflect {
        /** @class Object
         *
         *  @brief A swiss-army-knife base class for runtime polymorphism.
         *
         *  Promote using this:
         *  - for interpreter integration (see xo-expression / xo-jit)
         *  - to allow reasonably efficient type dispatching -
         *    don't need to pay for a function call to find out dispatching type.
         **/
        class Object : public reflect::SelfTagging {
        public:
            Object(TypeId type_id) : type_id_{type_id} {}

        private:
            /** unique id number for this object's type
             *
             *  Caches the value of this->self_tp().td()->id()
             *
             *  Notes:
             *  1. may want to record metatype also
             *  2. a few builtin types have well-known type_ids.
             *     see TypeDescrTable ctor in xo-reflect.
             **/
            TypeId type_id_;
        };

    } /*namespace obj*/
} /*namespace xo*/


/** end Object.hpp **/
