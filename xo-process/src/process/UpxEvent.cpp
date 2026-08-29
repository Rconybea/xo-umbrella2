/* @file UpxEvent.cpp */

#include "UpxEvent.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::reflect::StructReflector;
    using xo::pp::tostr;

    namespace process {
        UpxEvent::UpxEvent() = default;

        void
        UpxEvent::reflect_self()
        {
            StructReflector<UpxEvent> sr;

            if (sr.is_incomplete()) {
                REFLECT_MEMBER(sr, tm);
                REFLECT_MEMBER(sr, upx);
            }
        } /*reflect_self*/

        void
        UpxEvent::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            /* tm()/upx() return BY VALUE and field() holds a reference:
             * bind locals (pretty_struct.hpp lifetime rule)
             */
            const auto tm = this->tm();
            const auto upx = this->upx();

            sink.pretty_struct("UpxEvent",
                               field("tm", tm),
                               field("x", upx));
        }

        std::string
        UpxEvent::display_string() const {
            return tostr(*this);
        } /*display_string*/

    } /*namespace process*/
} /*namespace xo*/

/* end UpxEvent.cpp */
