/** @file ICollector_DMockCollector.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ICollector_DMockCollector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ICollector_DMockCollector.json5]
 **/

#pragma once

#include "Collector.hpp"
#include "../DMockCollector.hpp"

namespace xo { namespace mm { class ICollector_DMockCollector; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::ACollector,
                                   xo::mm::DMockCollector>
        {
            using ImplType = xo::mm::ICollector_Xfer
              <xo::mm::DMockCollector,
               xo::mm::ICollector_DMockCollector>;
        };
    }
}

namespace xo {
    namespace mm {
        /** @class ICollector_DMockCollector
         **/
        class ICollector_DMockCollector {
        public:
            /** @defgroup mm-collector-dmockcollector-type-traits **/
            ///@{
            using size_type = xo::mm::ACollector::size_type;
            using Copaque = xo::mm::ACollector::Copaque;
            using Opaque = xo::mm::ACollector::Opaque;
            using typeseq = xo::reflect::typeseq;
            ///@}
            /** @defgroup mm-collector-dmockcollector-methods **/
            ///@{
            // const methods
            /** memory in use for this collector **/
            static size_type allocated(const DMockCollector & self, Generation g, Role r) noexcept;
            /** memory committed for this collector **/
            static size_type committed(const DMockCollector & self, Generation g, Role r) noexcept;
            /** address space reserved for this collector **/
            static size_type reserved(const DMockCollector & self, Generation g, Role r) noexcept;
            /** Location of object in collector. -1 if not in collector memory.
Other negative values represent collector error states (good luck!).
Exact meaning of non-negative values up to collector implementation **/
            static std::int32_t locate_address(const DMockCollector & self, const void * addr) noexcept;
            /** true if gc responsible for data at @p addr, and data belongs to Role @p r **/
            static bool contains(const DMockCollector & self, Role r, const void * addr) noexcept;
            /** true iff gc-aware object of type @p tseq is installed in this collector **/
            static bool is_type_installed(const DMockCollector & self, typeseq tseq) noexcept;
            /** Report gc statistics, at discretion of collector implementation.
Creates dictionary using memory from @p report_mm.
If unable to comply (e.g. oom), return runtime error allocated from @p error_mm.
Avoiding obj<AGCObject> return type to avoid #include cycle **/
            static bool report_statistics(const DMockCollector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept;
            /** Report gc object types, at discretion of collector implementation.
Creates dictionary using memory from @p report_mm.
If unable to comply (e.g. oom), return runtime error allocated from @p error_mm.
Avoiding obj<AGCObject> return type to avoid #include cycle **/
            static bool report_object_types(const DMockCollector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept;
            /** Report gc object ages, at discretion of collector implementation.
Creates array of dictionaries using memory from @p report_mm.
Each dictionary has keys n-live and bytes, indexed by object age.
If unable to comply (e.g. oom), return runtime error allocated from @p error_mm.
Avoiding obj<AGCObject> return type to avoid #include cycle **/
            static bool report_object_ages(const DMockCollector & self, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output) noexcept;

            // non-const methods
            /** install interface @p iface for representation with typeseq @p tseq
in collector @p d.

The type AGCObject_Any here is misleading.
Will have been replaced by an instance of
  @c AGCObject_Xfer<DFoo,AGCObject_DFoo> for some @c DFoo
in which case calls through @c std::launder(&iface)
will properly act on @c DFoo.

Return false if installation fails (e.g. memory exhausted) **/
            static bool install_type(DMockCollector & self, const AGCObject & iface);
            /** add gc root with address @p p_root. gc will preserve subgraph at this address **/
            static void add_gc_root_poly(DMockCollector & self, obj<AGCObject> * p_root);
            /** remove gc root with address @p p_root. Reverse effect of prior add_gc_root_poly call **/
            static void remove_gc_root_poly(DMockCollector & self, obj<AGCObject> * p_root);
            /** Request immediate collection.
  1. if collection is enabled, immediately collect all generations
     up to (but not including) g
  2. may nevertheless escalate to older generations,
     depending on collector state.
  3. if collection is currently disabled,
     collection will trigger the next time gc is enabled.
 **/
            static void request_gc(DMockCollector & self, Generation upto);
            /** Assign pointer @p p_lhs to destination @p rhs, within parent allocation @p parent

Require: gc not in progress **/
            static void assign_member(DMockCollector & self, void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs);
            /** allocate copy of source object at address @p src.
Source must be owned by this collector.
Increments object age **/
            static void * alloc_copy(DMockCollector & self, std::byte * src);
            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end */
