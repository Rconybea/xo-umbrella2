/** @file RAbstractEventProcessor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/AbstractEventProcessor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/AbstractEventProcessor.json5]
 **/

#pragma once

#include "AAbstractEventProcessor.hpp"

namespace xo {
namespace reactor {

/** @class RAbstractEventProcessor
 **/
template <typename Object>
class RAbstractEventProcessor : public Object {
private:
    using O = Object;

public:
    /** @defgroup reactor-abstracteventprocessor-router-type-traits **/
    ///@{
    using ObjectType = Object;
    using DataPtr = Object::DataPtr;
    using typeseq = xo::reflect::typeseq;
    using obj_AAbstractEventProcessor = AAbstractEventProcessor::obj_AAbstractEventProcessor;
    using DString = AAbstractEventProcessor::DString;
    ///@}

    /** @defgroup reactor-abstracteventprocessor-router-ctors **/
    ///@{
    RAbstractEventProcessor() {}
    RAbstractEventProcessor(Object::DataPtr data) : Object{std::move(data)} {}
    RAbstractEventProcessor(const AAbstractEventProcessor * iface, void * data)
      requires std::is_same_v<typename Object::DataType, xo::facet::DVariantPlaceholder>
      : Object(iface, data) {}

    ///@}
    /** @defgroup reactor-abstracteventprocessor-router-methods **/
    ///@{

    // explicit injected content

    // builtin methods
    typeseq _typeseq() const noexcept { return O::iface()->_typeseq(); }
    void _drop() const noexcept { O::iface()->_drop(O::data()); }

    // const methods
    const DString * name()  const  noexcept {
        return O::iface()->name(O::data());
    }

    // non-const methods (still const in router!)
    void set_name(const DString * name)  noexcept {
        return O::iface()->set_name(O::data(), name);
    }

    ///@}
    /** @defgroup reactor-abstracteventprocessor-member-vars **/
    ///@{

    static bool _valid;

    ///@}
};

template <typename Object>
bool
RAbstractEventProcessor<Object>::_valid = xo::facet::valid_object_router<Object>();

} /*namespace reactor*/
} /*namespace xo*/

namespace xo { namespace facet {
    template <typename Object>
    struct RoutingFor<xo::reactor::AAbstractEventProcessor, Object> {
        using RoutingType = xo::reactor::RAbstractEventProcessor<Object>;
    };
} }

/* end RAbstractEventProcessor.hpp */
