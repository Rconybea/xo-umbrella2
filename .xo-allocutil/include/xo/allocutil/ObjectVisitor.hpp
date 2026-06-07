/** @file ObjectVisitor.hpp
 *
 **/

#pragma once

#include "IAlloc.hpp"
#include <cstdint>

namespace xo {
    namespace gc {
        /** @class ObjectVisitor
         *  @brief visit IObject* members of a T instance
         *
         *  Garbage collector relies on being able to navigate to
         *  an IObject-instance to find+update embedded pointers to
         *  other IObjects.
         *
         *  An IObject implemnetation must override
         *    IObject::_forward_children(IAlloc * gc)
         *  and call
         *   gc->forward_inplace(&child_)
         *  for each such child pointer.
         *
         *  For non-template classes this is straightforward
         *  See for example
         *    xo::obj::List in object/List.hpp
         *
         *  For a template class Foo<T> that contains T-instances,
         *  need to handle case where T contains IObject pointers.
         *  See for example the
         *    xo::tree::RedBlackTree in ordinaltree/RedBlackTree.hpp
         *
         *  Use ObjectVisitor<T>::forward_children() to pick up
         *  navigation code for such template arguments.
         **/
        template <typename T>
        class ObjectVisitor {
//        public:
//            void forward_children(T & target,
//                                  IAlloc * gc) { (void)target; (void)gc; }
        };

#define XO_TRIVIAL_OBJECT_VISITOR(TYPE)                       \
        template <>                                           \
        class ObjectVisitor<TYPE> {                           \
        public:                                               \
            static void forward_children(TYPE &, IAlloc *) {} \
        }

        XO_TRIVIAL_OBJECT_VISITOR(int32_t);
        XO_TRIVIAL_OBJECT_VISITOR(double);

    } /*namespace gc*/
} /*namespace xo*/

/* ObjectVisitor.hpp */
