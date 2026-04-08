/** @file DMockCollector.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/gc/GCObjectStore.hpp>
#include <xo/arena/AllocInfo.hpp>

namespace xo {
    namespace mm {

        /** @brief Mock Collector
         *
         *  Intended to help unit test a GCObjectSotre instance.
         *  Mock a Collector in collection phase for generations 0 <= g < @ref upto_.
         **/
        class DMockCollector {
        public:
            explicit DMockCollector(GCObjectStore * gcos, Generation upto) : p_gco_store_{gcos}, upto_{upto} {}

            template <typename AFacet>
            obj<AFacet,DMockCollector> ref() { return obj<AFacet,DMockCollector>(this); }

            Generation generation_of(Role r, const void * addr) const noexcept;
            AllocInfo alloc_info(void * mem) const noexcept;

            void visit_child(AGCObject * lhs_iface, void ** lhs_data);
            std::byte * alloc_copy(void * src) noexcept;

        private:
            GCObjectStore * p_gco_store_ = nullptr;
            Generation upto_;
        };

    } /*namespace mm*/
} /*namespaace xo*/

/* end DMockCollector.hpp */
