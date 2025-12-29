/** @file ISequence_DList.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/ISequence_DList.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/ISequence_DList.json5]
 **/

#pragma once

#include "Sequence.hpp"
#include "sequence/ISequence_Xfer.hpp"
#include "DList.hpp"

namespace xo { namespace scm { class ISequence_DList; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::scm::ASequence,
                                   xo::scm::DList>
        {
            using ImplType = xo::scm::ISequence_Xfer
              <xo::scm::DList,
               xo::scm::ISequence_DList>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class ISequence_DList
         **/
        class ISequence_DList {
        public:
            /** @defgroup scm-sequence-dlist-type-traits **/
            ///@{
            using size_type = ASequence::size_type;
            using AGCObject = ASequence::AGCObject;
            ///@}
            /** @defgroup scm-sequence-dlist-methods **/
            ///@{
            /** true iff sequence is empty **/
            static bool is_empty(const DList & self) noexcept;
            /** true iff sequence is finite **/
            static bool is_finite(const DList & self) noexcept;
            /** return element @p index of this sequence **/
            static obj<AGCObject> at(const DList & self, size_type index);

            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */