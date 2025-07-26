/** @file TypeBlueprint.cpp **/

#include "typeinf/TypeBlueprint.hpp"

namespace xo {
    namespace scm {
        TypeBlueprint::TypeBlueprint(const type_ref & x)
            : ref_{x}
        {}

        rp<TypeBlueprint>
        TypeBlueprint::make(const type_ref & ref)
        {
            return new TypeBlueprint(ref);
        }

        rp<TypeBlueprint>
        TypeBlueprint::typevar(const type_var & name)
        {
            return new TypeBlueprint(type_ref(name, nullptr));
        }

        bool
        TypeBlueprint::equals(bp<TypeBlueprint> lhs, bp<TypeBlueprint> rhs)
        {
            // 1. two concrete blueprints are equal if they resolve to the same type.
            // 2. two type variables are equal if they have the same unique name;
            //    but: once we introduce structural constraints will relax this

            if (lhs->is_concrete() && rhs->is_concrete())
            {
                return lhs->td() == rhs->td();
            }

            if (lhs->id() == rhs->id())
            {
                // typevar names are globally unique,
                // so two typevars with the same name must refer to the same type
                return true;
            }

            // TODO: structural comparisons..

            return false;
        }

        bool
        TypeBlueprint::is_variable() const
        {
            // TODO;
            // if we have structural information about this type,
            // e.g. vector[t'] or function(a' -> b'),
            // then must return false here

            return !ref_.is_concrete();
        }

        void
        TypeBlueprint::upsert_typevars(std::set<type_var> * p_typevar_set) const
        {
            if (this->is_concrete()) {
                return;
            }

            // TODO: handle structural types

            p_typevar_set->insert(ref_.id());
        }
        bp<TypeBlueprint>
        TypeBlueprint::substitute(const type_substitution_map& sub_map)
        {
            bp<TypeBlueprint> subject = this;

            // loop here should only run once.
            // we collapse sub_map whenever we extend it.
            //
            while(!subject->is_concrete()) {
                auto ix = sub_map.find(subject->id());

                if (ix == sub_map.end())
                    break;

                subject = ix->second.get();
            }

            // TODO: also want to update the whole chain,
            //       so that everything refers to final subjectc

            return subject;
        }

        void
        TypeBlueprint::resolve_to(TypeDescr td)
        {
            ref_.resolve_to(td);
        }

        void
        TypeBlueprint::display(std::ostream & os) const
        {
            os << "<TypeBlueprint";
            os << xtag("id", id());
            if (td())
                os << xtag("td", td()->canonical_name());
            os << ">";
        }

    } /*namespace scm*/
} /*namespace xo*/


/** end TypeBlueprint.cpp **/
