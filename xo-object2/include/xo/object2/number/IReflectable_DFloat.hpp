/** @file IReflectable_DFloat.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IReflectable_DFloat.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IReflectable_DFloat.json5]
 **/

#pragma once

#include "Reflectable.hpp"
#include <xo/reflectable2/Reflectable.hpp>
#include <xo/reflectable2/detail/IReflectable_Xfer.hpp>
#include "DFloat.hpp"

namespace xo { namespace scm { class IReflectable_DFloat; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::reflect::AReflectable,
                                   xo::scm::DFloat>
        {
            using ImplType = xo::reflect::IReflectable_Xfer
              <xo::scm::DFloat,
               xo::scm::IReflectable_DFloat>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IReflectable_DFloat
         **/
        class IReflectable_DFloat {
        public:
            /** @defgroup scm-reflectable-dfloat-type-traits **/
            ///@{
            using obj_AReflectable = xo::reflect::AReflectable::obj_AReflectable;
            using TaggedPtr = xo::reflect::AReflectable::TaggedPtr;
            using Copaque = xo::reflect::AReflectable::Copaque;
            using Opaque = xo::reflect::AReflectable::Opaque;
            ///@}
            /** @defgroup scm-reflectable-dfloat-methods **/
            ///@{
            // const methods

            // non-const methods
            /** TaggedPtr for this object's concrete representation **/
            static TaggedPtr self_tp(DFloat & self);
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */