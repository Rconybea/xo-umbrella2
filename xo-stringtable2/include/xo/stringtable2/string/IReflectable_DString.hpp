/** @file IReflectable_DString.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IReflectable_DString.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IReflectable_DString.json5]
 **/

#pragma once

#include "Reflectable.hpp"
#include <xo/reflectable2/Reflectable.hpp>
#include <xo/reflectable2/detail/IReflectable_Xfer.hpp>
#include "DString.hpp"

namespace xo { namespace scm { class IReflectable_DString; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::reflect::AReflectable,
                                   xo::scm::DString>
        {
            using ImplType = xo::reflect::IReflectable_Xfer
              <xo::scm::DString,
               xo::scm::IReflectable_DString>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IReflectable_DString
         **/
        class IReflectable_DString {
        public:
            /** @defgroup scm-reflectable-dstring-type-traits **/
            ///@{
            using obj_AReflectable = xo::reflect::AReflectable::obj_AReflectable;
            using TaggedPtr = xo::reflect::AReflectable::TaggedPtr;
            using Copaque = xo::reflect::AReflectable::Copaque;
            using Opaque = xo::reflect::AReflectable::Opaque;
            ///@}
            /** @defgroup scm-reflectable-dstring-methods **/
            ///@{
            // const methods

            // non-const methods
            /** TaggedPtr for this object's concrete representation **/
            static TaggedPtr self_tp(DString & self);
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */