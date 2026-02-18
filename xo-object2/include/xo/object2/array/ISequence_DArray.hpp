/** @file ISequence_DArray.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ISequence_DArray.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ISequence_DArray.json5]
 **/

#pragma once

#include "Sequence.hpp"
#include "DArray.hpp"

namespace xo { namespace scm { class ISequence_DArray; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::scm::ASequence,
                                   xo::scm::DArray>
        {
            using ImplType = xo::scm::ISequence_Xfer
              <xo::scm::DArray,
               xo::scm::ISequence_DArray>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class ISequence_DArray
         **/
        class ISequence_DArray {
        public:
            /** @defgroup scm-sequence-darray-type-traits **/
            ///@{
            using size_type = xo::scm::ASequence::size_type;
            using AGCObject = xo::scm::ASequence::AGCObject;
            using Copaque = xo::scm::ASequence::Copaque;
            using Opaque = xo::scm::ASequence::Opaque;
            ///@}
            /** @defgroup scm-sequence-darray-methods **/
            ///@{
            // const methods
            /** true iff sequence is empty **/
            static bool is_empty(const DArray & self) noexcept;
            /** true iff sequence is finite **/
            static bool is_finite(const DArray & self) noexcept;
            /** return element @p index of this sequence **/
            static obj<AGCObject> at(const DArray & self, size_type index);

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */