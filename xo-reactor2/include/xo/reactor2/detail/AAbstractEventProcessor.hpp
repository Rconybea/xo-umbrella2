/** @file AAbstractEventProcessor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/AbstractEventProcessor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/AbstractEventProcessor.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/stringtable2/DString.hpp>
#include <xo/facet/ATop.hpp>
#include <xo/facet/obj.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/facet/typeseq.hpp>

namespace xo::scm { class DString; }

namespace xo {
namespace reactor {

using Copaque = const void *;
using Opaque = void *;

/**
Trait for data types representing nodes in a computational dataflow graph.
**/
class AAbstractEventProcessor : public xo::facet::ATop {
public:
    /** @defgroup reactor-abstracteventprocessor-type-traits **/
    ///@{
    // types
    /** integer identifying a type **/
    using typeseq = xo::facet::typeseq;
    using Copaque = const void *;
    using Opaque = void *;
    /**  **/
    using obj_AAbstractEventProcessor = xo::facet::obj<AAbstractEventProcessor>;
    /** string representation **/
    using DString = xo::scm::DString;
    ///@}

    /** @defgroup reactor-abstracteventprocessor-methods **/
    ///@{
    // const methods
    /* _has_null_vptr(), _typeseq(), _drop(): inherited from xo::facet::ATop */
    /** Reporting name for this node.  Ideally (but not necessarily) unique **/
    virtual const DString * name(Copaque data)  const  noexcept = 0;

    // nonconst methods
    /** Assign reporting name for this node. **/
    virtual void set_name(Opaque data, const DString * name)  noexcept = 0;
    ///@}
}; /*AAbstractEventProcessor*/

/** Implementation IAbstractEventProcessor_DRepr of AAbstractEventProcessor for state DRepr
 *  should provide a specialization:
 *
 *    template <>
 *    struct xo::facet::FacetImplementation<AAbstractEventProcessor, DRepr> {
 *        using Impltype = IAbstractEventProcessor_DRepr;
 *    };
 *
 *  then IAbstractEventProcessor_ImplType<DRepr> --> IAbstractEventProcessor_DRepr
 **/
template <typename DRepr>
using IAbstractEventProcessor_ImplType = xo::facet::FacetImplType<AAbstractEventProcessor, DRepr>;

} /*namespace reactor*/
} /*namespace xo*/

/* AAbstractEventProcessor.hpp */
