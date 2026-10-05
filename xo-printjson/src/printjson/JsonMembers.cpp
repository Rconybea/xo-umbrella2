/** @file JsonMembers.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonMembers.hpp"
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cassert>
#include <exception>

namespace xo {
    using xo::pp::quot;
    using xo::reflect::Metatype;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;

    namespace json {
        JsonMembers::JsonMembers(JsonPrintState & state)
            : state_{&state}, p_os_{state.p_os()}, n_uncaught_{std::uncaught_exceptions()}
        {
            /* the leading comma: the object's _name_ and type keys come first */
            *p_os_ << ", \"_members_\": [";
        }

        JsonMembers::~JsonMembers()
        {
            assert((ended_ || std::uncaught_exceptions() > n_uncaught_)
                   && "JsonMembers: end() not called");
        }

        void
        JsonMembers::end()
        {
            *p_os_ << "]";
            ended_ = true;
        }

        bool
        JsonMembers::printable(TypeDescr td) const
        {
            return (state_->has_printer(td)
                    || (td->is_struct() && td->complete_flag()));
        }

        void
        JsonMembers::write_head(std::string_view name, DeclaredType const & declared)
        {
            if (!first_)
                *p_os_ << ", ";
            first_ = false;

            *p_os_ << "{" << quot("_name_") << ": " << quot(name)
                   << ", " << type_keys(declared.canonical_, declared.short_)
                   << ", " << quot("_metatype_") << ": " << quot(metatype2str(declared.metatype_));
        }

        void
        JsonMembers::write_value(std::string_view name, DeclaredType const & declared,
                                 TaggedPtr value, bool identity)
        {
            this->write_head(name, declared);

            *p_os_ << ", " << quot("_value_") << ": ";
            if (identity)
                state_->print(value);
            else
                state_->print_value(value);
            *p_os_ << "}";
        }

        void
        JsonMembers::write_ref(std::string_view name, DeclaredType const & declared,
                               void const * p)
        {
            this->write_head(name, declared);

            *p_os_ << ", " << quot("_value_") << ": ";
            this->write_ref_value(p);
            *p_os_ << "}";
        }

        void
        JsonMembers::write_refs(std::string_view name, DeclaredType const & declared,
                                std::vector<void const *> const & ps)
        {
            this->write_head(name, declared);

            *p_os_ << ", " << quot("_value_") << ": [";
            for (std::size_t i = 0, n = ps.size(); i < n; ++i) {
                if (i > 0)
                    *p_os_ << ", ";
                this->write_ref_value(ps[i]);
            }
            *p_os_ << "]}";
        }

        void
        JsonMembers::write_ref_map(std::string_view name, DeclaredType const & declared,
                                   std::vector<std::pair<std::string, void const *>> const & kvs)
        {
            this->write_head(name, declared);

            *p_os_ << ", " << quot("_value_") << ": {";
            for (std::size_t i = 0, n = kvs.size(); i < n; ++i) {
                if (i > 0)
                    *p_os_ << ", ";
                *p_os_ << quot(kvs[i].first) << ": ";
                this->write_ref_value(kvs[i].second);
            }
            *p_os_ << "}}";
        }

        void
        JsonMembers::write_ref_value(void const * p)
        {
            state_->print_ref(p);
        }

        void
        JsonMembers::write_error(std::string_view name, DeclaredType const & declared,
                                 std::string const & why)
        {
            this->write_head(name, declared);

            *p_os_ << ", " << quot("_error_") << ": " << quot(why) << "}";
        }
    } /*namespace json*/
} /*namespace xo*/

/* end JsonMembers.cpp */
