/** @file DMockCollector.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/gc/MutationLogStore.hpp>
#include <xo/gc/GCObjectStore.hpp>

namespace xo {
    namespace mm {

        class DMockCollector {
        public:
            using size_type = GCObjectStore::size_type;
            using typeseq = xo::facet::typeseq;

            DMockCollector(MutationLogStore * mls, GCObjectStore * gcos)
            : mls_{mls}, gcos_{gcos} {}

            size_type allocated(Generation g, Role r) const noexcept;
            size_type committed(Generation g, Role r) const noexcept;
            size_type reserved(Generation g, Role r) const noexcept;

            // like generation_fo(), but for ACollector api
            int32_t locate_address(const void * addr) const noexcept;

            // true iff gcos contains address @p addr in @p role
            bool contains(Role r, const void * addr) const noexcept;

            // true iff @p tseq has been installed in @p gcos_
            bool is_type_installed(typeseq tseq) const noexcept;

            bool report_statistics(obj<AAllocator> mm,
                                   obj<AAllocator> error_mm,
                                   obj<AGCObject> * p_output) const noexcept;

            bool report_object_types(obj<AAllocator> mm,
                                     obj<AAllocator> error_mm,
                                     obj<AGCObject> * p_output) const noexcept;

            bool report_object_ages(obj<AAllocator> mm,
                                     obj<AAllocator> error_mm,
                                     obj<AGCObject> * p_output) const noexcept;

            bool install_type(const AGCObject & meta) noexcept;

            void add_gc_root_poly(obj<AGCObject> * p_root);
            void remove_gc_root_poly(obj<AGCObject> * p_root);
            void request_gc(Generation upto);

            // write barrier for assignment
            void assign_member(void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs);

            void * alloc_copy(std::byte * src);

            MutationLogStore * mls_ = nullptr;
            GCObjectStore * gcos_ = nullptr;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end DMockCollector.hpp */
