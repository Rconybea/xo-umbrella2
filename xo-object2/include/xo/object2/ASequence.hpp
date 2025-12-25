/** @file ASequence.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2/xo-object2/../xo-facet/codegen/genfacet.py]
 *     arguments:
 *       --input [./idl/Sequence.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [abstract_facet.hpp.j2]
 *  3. idl for facet methods
 *       [./idl/Sequence.json5]
 **/

#pragma once

// includes (via {facet_includes})
#include <xo/gc/GCObject.hpp>

namespace xo {
namespace scm {

/**
Elements appear in some determinstic order.
Sequence is GC-aware --> elements must be GC-aware
**/
class ASequence {
public:

    /** true iff sequence is empty **/
    virtual bool is_empty() const noexcept = 0;


    /** true iff sequence is finite **/
    virtual bool is_finite() const noexcept = 0;


    /** return element @p index of this sequence **/
    virtual obj<AGCObject> at(size_type index) const = 0;

}; /*ASequence*/

} /*namespace scm*/
} /*namespace xo*/