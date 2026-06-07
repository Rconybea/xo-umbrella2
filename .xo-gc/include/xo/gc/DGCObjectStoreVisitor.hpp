/** @file DGCObjectStoreVisitor.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/VisitReason.hpp>
#include <xo/alloc2/Generation.hpp>
#include <xo/alloc2/role.hpp>
#include <xo/arena/AllocInfo.hpp>
#include <xo/facet/obj.hpp>

namespace xo {
    namespace mm {

        class GCObjectStore; // see GCObjectStore.hpp
        class AGCObject; // see AGCObject.hpp

        /** @brief visitor shim for GCObjectStore
         *
         *  For a GC cycle, remembers which generations
         *  are being collected
         **/
        class DGCObjectStoreVisitor {
        public:
            DGCObjectStoreVisitor(GCObjectStore * gcos, Generation upto);

            template <typename AFacet = AGCObjectVisitor>
            obj<AFacet,DGCObjectStoreVisitor> ref() { return obj<AFacet,DGCObjectStoreVisitor>(this); }

            Generation generation_of(Role r, const void * addr) const noexcept;
            AllocInfo alloc_info(void * mem) const noexcept;

            void visit_child(VisitReason reason, AGCObject * lhs_iface, void ** lhs_data);
            std::byte * alloc_copy(void * src) noexcept;

        private:
            /** object storage **/
            GCObjectStore * p_gco_store_ = nullptr;
            /** collecting generations up to this bound **/
            Generation upto_;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end DGCObjectStoreVisitor.hpp */
