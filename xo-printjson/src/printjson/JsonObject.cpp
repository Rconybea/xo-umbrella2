/** @file JsonObject.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonObject.hpp"
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cassert>

namespace xo {
    using xo::pp::quot;
    using xo::reflect::TaggedPtr;

    namespace json {
        JsonObject::~JsonObject()
        {
            assert((closed_ || std::uncaught_exceptions() > n_uncaught_)
                   && "JsonObject: close() not called");
        }

        void
        JsonObject::write_key(std::string_view k)
        {
            *state_->p_os() << ", " << quot(k) << ": ";
        }

        JsonObject &
        JsonObject::child(std::string_view k, TaggedPtr tp)
        {
            this->write_key(k);
            state_->print(tp);

            return *this;
        }

        JsonObject &
        JsonObject::key_ref(std::string_view k, void const * p)
        {
            this->write_key(k);
            state_->print_ref(p);

            return *this;
        }

        std::ostream &
        JsonObject::key_open(std::string_view k)
        {
            this->write_key(k);

            return *state_->p_os();
        }

        JsonMembers
        JsonObject::members()
        {
            return JsonMembers(*state_);
        }

        void
        JsonObject::close()
        {
            *state_->p_os() << "}";
            closed_ = true;
        }
    } /*namespace json*/
} /*namespace xo*/

/* end JsonObject.cpp */
