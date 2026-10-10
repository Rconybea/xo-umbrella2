/** @file JsonMembers.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonMembers.hpp"
#include <xo/reflect/struct/StructMember.hpp>
#include <xo/reflect/atomic/StdAtomicTdx.hpp>
#include <xo/reflect/wrapper/WrapperTdx.hpp>
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cassert>
#include <exception>

namespace xo {
    using xo::pp::quot;
    using xo::reflect::Metatype;
    using xo::reflect::Ownership;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;

    namespace json {
        /* one scope in from namespace xo: see PrintJson.cpp */
        using xo::pp::tostr;

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
            if (reflect::StdAtomicTdx const * ai = td->std_atomic_info()) {
                /* a std::atomic<T> prints its load()ed T.  (An atomic
                 * pointer would need its target, which a type cannot say:
                 * not printable)
                 */
                return this->printable(ai->value_td());
            }

            if (reflect::WrapperTdx const * wi = td->wrapper_info()) {
                /* a transparent wrapper prints as its wrapped value */
                return this->printable(wi->wrapped_td());
            }

            return (state_->has_printer(td)
                    || (td->is_struct() && td->complete_flag())
                    || td->is_enum());
        }

        bool
        JsonMembers::printable_value(TaggedPtr v) const
        {
            TypeDescr td = v.td();

            if (!td)
                return false;

            /* as member_target<V> unwraps a declared type: a printer for
             * the wrapper itself wins, as it would in print()
             */
            if (state_->has_printer(td))
                return true;

            switch (td->metatype()) {
            case Metatype::mt_pointer:
                /* borrowed: a ref, or null -- printable without reading the
                 * pointee (.xo-backlog/xo-printjson/issues/08)
                 */
                if (td->child_edge_ownership() == Ownership::borrowed)
                    return true;
                if (v.n_child() == 0)
                    return true;
                {
                    TaggedPtr target = v.get_child(0);

                    /* printed already: a ref, which needs no printer for
                     * its type -- e.g. a receiver an endpoint writes inline
                     * (.xo-backlog/xo-reflect/issues/04)
                     */
                    return state_->is_printed(target) || this->printable_value(target);
                }
            case Metatype::mt_vector:
                return (v.n_child() == 0) || this->printable_value(v.get_child(0));
            default:
                return this->printable(td);
            }
        }

        JsonMembers &
        JsonMembers::reflected_members(TaggedPtr obj, std::string_view name_suffix)
        {
            TypeDescr td = obj.td();

            if (!td || !td->is_struct())
                return *this;

            for (std::uint32_t i = 0, n = obj.n_child(); i < n; ++i) {
                reflect::StructMember const & sm = td->struct_member(i);
                TypeDescr mtd = sm.get_member_td();
                DeclaredType declared{mtd->canonical_name(), std::string(mtd->short_name()),
                                      mtd->metatype()};
                TaggedPtr value = sm.get_member_tp(obj.address());
                std::string name = tostr(sm.member_name(), name_suffix);

                if (mtd->is_pointer() && !state_->has_printer(mtd)) {
                    /* the edge to its pointee as this struct's reflection
                     * says (StructMember::ownership): it may override the
                     * pointer type's
                     */
                    Ownership edge = sm.ownership();

                    if ((edge == Ownership::borrowed) || this->printable_value(value)) {
                        this->write_pointee(name, declared, value, edge);
                    } else {
                        this->write_error(name, declared,
                                          tostr("type not reflected: ", mtd->canonical_name()));
                    }
                } else if (this->printable_value(value)) {
                    this->write_value(name, declared, value, true /*identity*/);
                } else {
                    this->write_error(name, declared,
                                      tostr("type not reflected: ", mtd->canonical_name()));
                }
            }

            return *this;
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
        JsonMembers::write_pointee(std::string_view name, DeclaredType const & declared,
                                   TaggedPtr ptr, Ownership edge)
        {
            this->write_head(name, declared);

            *p_os_ << ", " << quot("_value_") << ": ";
            state_->print_pointee(ptr, edge);
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
