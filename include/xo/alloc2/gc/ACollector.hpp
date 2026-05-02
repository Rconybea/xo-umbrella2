/** @file ACollector.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/Collector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/Collector.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/alloc2/Allocator_basic.hpp>
#include <xo/alloc2/Generation.hpp>
#include <xo/alloc2/role.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo { namespace mm { class AGCObject; } }
namespace xo { namespace mm { class IGCObject_Any; } }
// more pretext here..

namespace xo {
namespace mm {

using Copaque = const void *;
using Opaque = void *;

/**
A collector must also suppose the @ref AAllocator facet, see also
**/
class ACollector {
public:
    /** @defgroup mm-collector-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /** allocation size type **/
    using size_type = std::size_t;
    ///@}

    /** @defgroup mm-collector-methods **/
    ///@{
    // const methods
    /** An uninitialized ACollector instance will have zero vtable pointer (per {linux,osx} abi).
     *  Use case for this is narrow. We go to some lengths to avoid null vtable pointers. For example
     *  obj<AFacet> will have non-null vtable (via IFacet_Any) with all methods terminating.
     **/
    bool _has_null_vptr() const noexcept { return *reinterpret_cast<const void * const *>(this) == nullptr; }
    /** RTTI: unique id# for actual runtime data representation **/
    virtual typeseq _typeseq() const noexcept = 0;
    /** destroy instance @p d; calls c++ dtor only for actual runtime type; does not recover memory **/
    virtual void _drop(Opaque d) const noexcept = 0;
    /** memory in use for this collector **/
    virtual size_type allocated(Copaque data, Generation g, Role r)  const  noexcept = 0;
    /** memory committed for this collector **/
    virtual size_type committed(Copaque data, Generation g, Role r)  const  noexcept = 0;
    /** address space reserved for this collector **/
    virtual size_type reserved(Copaque data, Generation g, Role r)  const  noexcept = 0;
    /** Location of object in collector. -1 if not in collector memory.
Other negative values represent collector error states (good luck!).
Exact meaning of non-negative values up to collector implementation **/
    virtual std::int32_t locate_address(Copaque data, const void * addr)  const  noexcept = 0;
    /** true if gc responsible for data at @p addr, and data belongs to Role @p r **/
    virtual bool contains(Copaque data, Role r, const void * addr)  const  noexcept = 0;
    /** true iff gc-aware object of type @p tseq is installed in this collector **/
    virtual bool is_type_installed(Copaque data, typeseq tseq)  const  noexcept = 0;
    /** Report gc statistics, at discretion of collector implementation.
Creates dictionary using memory from @p report_mm.
If unable to comply (e.g. oom), return runtime error allocated from @p error_mm.
Avoiding obj<AGCObject> return type to avoid #include cycle **/
    virtual bool report_statistics(Copaque data, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept = 0;
    /** Report gc object types, at discretion of collector implementation.
Creates dictionary using memory from @p report_mm.
If unable to comply (e.g. oom), return runtime error allocated from @p error_mm.
Avoiding obj<AGCObject> return type to avoid #include cycle **/
    virtual bool report_object_types(Copaque data, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept = 0;
    /** Report gc object ages, at discretion of collector implementation.
Creates array of dictionaries using memory from @p report_mm.
Each dictionary has keys n-live and bytes, indexed by object age.
If unable to comply (e.g. oom), return runtime error allocated from @p error_mm.
Avoiding obj<AGCObject> return type to avoid #include cycle **/
    virtual bool report_object_ages(Copaque data, obj<AAllocator> report_mm, obj<AAllocator> error_mm, obj<AGCObject> * output)  const  noexcept = 0;

    // nonconst methods
    /** install interface @p iface for representation with typeseq @p tseq
in collector @p d.

The type AGCObject_Any here is misleading.
Will have been replaced by an instance of
  @c AGCObject_Xfer<DFoo,AGCObject_DFoo> for some @c DFoo
in which case calls through @c std::launder(&iface)
will properly act on @c DFoo.

Return false if installation fails (e.g. memory exhausted) **/
    virtual bool install_type(Opaque data, const AGCObject & iface)  = 0;
    /** add gc root with address @p p_root. gc will preserve subgraph at this address **/
    virtual void add_gc_root_poly(Opaque data, obj<AGCObject> * p_root)  = 0;
    /** remove gc root with address @p p_root. Reverse effect of prior add_gc_root_poly call **/
    virtual void remove_gc_root_poly(Opaque data, obj<AGCObject> * p_root)  = 0;
    /** Request immediate collection.
  1. if collection is enabled, immediately collect all generations
     up to (but not including) g
  2. may nevertheless escalate to older generations,
     depending on collector state.
  3. if collection is currently disabled,
     collection will trigger the next time gc is enabled.
 **/
    virtual void request_gc(Opaque data, Generation upto)  = 0;
    /** Assign pointer @p p_lhs to destination @p rhs, within parent allocation @p parent
DEPRECATED. Only used in MockCollector for gc unit test

Require: gc not in progress **/
    virtual void assign_member(Opaque data, void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> & rhs)  = 0;
    /** allocate copy of source object at address @p src.
Source must be owned by this collector.
Increments object age **/
    virtual void * alloc_copy(Opaque data, std::byte * src)  = 0;
    ///@}
}; /*ACollector*/

/** Implementation ICollector_DRepr of ACollector for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<ACollector, DRepr> {
 *        using Impltype = ICollector_DRepr;
 *    };
 *
 *  then ICollector_ImplType<DRepr> --> ICollector_DRepr
 **/
template <typename DRepr>
using ICollector_ImplType = xo::facet::FacetImplType<ACollector, DRepr>;

} /*namespace mm*/
} /*namespace xo*/

/* ACollector.hpp */
