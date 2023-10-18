/* @file UpxEvent.cpp */

#include "UpxEvent.hpp"
#include "xo/reflect/StructReflector.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
    using xo::reflect::StructReflector;
    using xo::tostr;
    using xo::xtag;

    namespace process {
        UpxEvent::UpxEvent() = default;

        void
        UpxEvent::reflect_self()
        {
            StructReflector<UpxEvent> sr;

            if (sr.is_incomplete()) {
                //REFLECT_MEMBER(sr, contents);
                REFLECT_MEMBER(sr, tm);
                REFLECT_MEMBER(sr, upx);
            }
        } /*reflect_self*/

        void
        UpxEvent::display(std::ostream & os) const
        {
            os << "<UpxEvent"
               << xtag("tm", this->tm())
               << xtag("x", this->upx())
               << ">";
        } /*display*/

        std::string
        UpxEvent::display_string() const {
            return tostr(*this);
        } /*display_string*/

    } /*namespace process*/
} /*namespace xo*/

/* end UpxEvent.cpp */
