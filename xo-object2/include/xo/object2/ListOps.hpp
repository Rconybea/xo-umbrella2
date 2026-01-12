/** @file ListOps.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "IGCObject_DList.hpp"
#include "DList.hpp"

namespace xo {
    namespace scm {
        /** @brief list functions
         *
         *  note: separate from DList, to avoid problems with deps needed
         *        to compile functions that return obj<AGCObject,DList>
         **/
        struct ListOps {
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;

            template <typename AConsFacet = AGCObject>
            static obj<AConsFacet,DList> nil();

            /** shortcut for
             *    cons(mm, cdr, cdr.data())
             **/
            template <typename AConsFacet = AGCObject, typename ACdrFacet = AGCObject>
            static obj<AConsFacet,DList> cons(obj<AAllocator> mm,
                                              obj<AGCObject> car,
                                              obj<ACdrFacet,DList> cdr);

            /** list with one element @p e1, allocated from @p mm **/
            template <typename AListFacet = AGCObject>
            static obj<AListFacet,DList> list(obj<AAllocator> mm,
                                              obj<AGCObject> e1);

            /** list with two element @p e1, @p e2, allocated from @p mm **/
            template <typename AListFacet = AGCObject>
            static obj<AListFacet,DList> list(obj<AAllocator> mm,
                                              obj<AGCObject> e1,
                                              obj<AGCObject> e2);

        };

        template <typename AConsFacet>
        obj<AConsFacet,DList>
        ListOps::nil()
        {
            return obj<AConsFacet,DList>(DList::_nil());
        }

        template <typename AConsFacet, typename ACdrFacet>
        obj<AConsFacet,DList>
        ListOps::cons(obj<AAllocator> mm,
                    obj<AGCObject> car,
                    obj<ACdrFacet,DList> cdr)
        {
            return obj<AConsFacet,DList>(DList::_cons(mm, car, cdr.data()));
        }

        template <typename AListFacet>
        obj<AListFacet,DList>
        ListOps::list(obj<AAllocator> mm,
                    obj<AGCObject> e1)
        {
            // clang 15 doesn't like nil() here.

            return cons(mm, e1, nil());
        }

        template <typename AListFacet>
        obj<AListFacet,DList>
        ListOps::list(obj<AAllocator> mm,
                    obj<AGCObject> e1,
                    obj<AGCObject> e2)
        {
            return cons(mm, e1, list(mm, e2));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end ListOps.hpp */
