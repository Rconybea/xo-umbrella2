/** @file StreamReceiver.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "StreamReceiver.hpp"
#include <xo/reflect/StructReflector.hpp>

namespace xo {
    using xo::reflect::StructReflector;

    namespace web {
        void
        StreamReceiver::reflect_self(reflect::TypeDescrTable * /*table*/)
        {
            /* no members: an interface.  Self-tagging, so a pointer to one
             * reflects to the actual receiver
             */
            StructReflector<StreamReceiver> sr;
        } /*reflect_self*/
    } /*namespace web*/
} /*namespace xo*/

/* end StreamReceiver.cpp */
