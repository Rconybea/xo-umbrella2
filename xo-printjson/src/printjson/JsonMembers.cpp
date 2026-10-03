/** @file JsonMembers.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonMembers.hpp"
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cassert>
#include <sstream>

namespace xo {
    using xo::pp::quot;
    using xo::reflect::Metatype;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;

    namespace json {
        std::string
        json_id(void const * p)
        {
            std::ostringstream ss;
            ss << p;
            return ss.str();
        }

        JsonMembers::JsonMembers(PrintJson const * pjson, std::ostream * p_os)
            : pjson_{pjson}, p_os_{p_os}
        {
            /* the leading comma: _name_ and _type_ always come first */
            *p_os_ << ", \"_members_\": [";
        }

        JsonMembers::~JsonMembers()
        {
            assert(ended_ && "JsonMembers: end() not called");
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
            return (pjson_->has_printer(td)
                    || (td->is_struct() && td->complete_flag()));
        }

        void
        JsonMembers::write_head(std::string_view name, std::string const & declared,
                                Metatype metatype)
        {
            if (!first_)
                *p_os_ << ", ";
            first_ = false;

            *p_os_ << "{" << quot("_name_") << ": " << quot(name)
                   << ", " << quot("_type_") << ": " << quot(declared)
                   << ", " << quot("_metatype_") << ": " << quot(metatype2str(metatype));
        }

        void
        JsonMembers::write_value(std::string_view name, std::string const & declared,
                                 Metatype metatype, TaggedPtr value)
        {
            this->write_head(name, declared, metatype);

            *p_os_ << ", " << quot("_value_") << ": ";
            pjson_->print_aux(value, p_os_);
            *p_os_ << "}";
        }

        void
        JsonMembers::write_ref(std::string_view name, std::string const & declared,
                               Metatype metatype, void const * p)
        {
            this->write_head(name, declared, metatype);

            *p_os_ << ", " << quot("_value_") << ": ";
            this->write_ref_value(p);
            *p_os_ << "}";
        }

        void
        JsonMembers::write_refs(std::string_view name, std::string const & declared,
                                Metatype metatype, std::vector<void const *> const & ps)
        {
            this->write_head(name, declared, metatype);

            *p_os_ << ", " << quot("_value_") << ": [";
            for (std::size_t i = 0, n = ps.size(); i < n; ++i) {
                if (i > 0)
                    *p_os_ << ", ";
                this->write_ref_value(ps[i]);
            }
            *p_os_ << "]}";
        }

        void
        JsonMembers::write_ref_map(std::string_view name, std::string const & declared,
                                   Metatype metatype,
                                   std::vector<std::pair<std::string, void const *>> const & kvs)
        {
            this->write_head(name, declared, metatype);

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
            if (p)
                *p_os_ << "{" << quot("ref") << ": " << quot(json_id(p)) << "}";
            else
                *p_os_ << "null";
        }

        void
        JsonMembers::write_error(std::string_view name, std::string const & declared,
                                 Metatype metatype, std::string const & why)
        {
            this->write_head(name, declared, metatype);

            *p_os_ << ", " << quot("_error_") << ": " << quot(why) << "}";
        }
    } /*namespace json*/
} /*namespace xo*/

/* end JsonMembers.cpp */
