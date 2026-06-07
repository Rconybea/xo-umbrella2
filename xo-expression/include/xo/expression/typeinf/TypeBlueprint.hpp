/** @file TypeBlueprint.hpp **/

#include "xo/refcnt/Refcounted.hpp"
#include "type_ref.hpp"
#include <map>
#include <set>

namespace xo {
    namespace scm {
        class TypeBlueprint;

        /** map from a type variable, to contraints on the resolution of that variable **/
        using type_substitution_map = std::map<type_var, rp<TypeBlueprint>>;

        /** @class TypeBlueprint
         *  @brief record constraints on a type variable.
         *
         *  Within type unification, a TypeBlueprint represents
         *  current state of knowledge as to the resolution of a particular type.
         *
         *  Structurally homologous to @ref xo::reflect::TypeDescr,
         *  but TypeDescr is intended to represent fully-defined types.
         *  Conversely TypeBlueprint instances will be abandoned once
         *  a corresponding TypeDescr exists.
         **/
        class TypeBlueprint : public xo::ref::Refcount {
        public:
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            TypeBlueprint() = default;

            /** contruct blueprint for type_ref @p ref **/
            static rp<TypeBlueprint> make(const type_ref& ref);
            /** contruct blueprint for type variable @p name.
             *  equivalent to @c make(type_ref(name, nullptr))
             **/
            static rp<TypeBlueprint> typevar(const type_var& name);

            /** compare two blueprints for equality.
             *  blueprints are equal iff (we know that) they refer to the same concrete type.
             **/
            static bool equals(bp<TypeBlueprint> lhs, bp<TypeBlueprint> rhs);

            const type_ref& ref() const { return ref_; }
            const type_var& id() const { return ref_.id(); }
            TypeDescr td() const { return ref_.td(); }

            bool is_concrete() const { return ref_.is_concrete(); }
            bool is_variable() const;

            /** upsert into @p *p_typevarset all unresolved type variables **/
            void upsert_typevars(std::set<type_var> * p_typevar_set) const;

            /** apply substitutions in @p sub_map to this type **/
            bp<TypeBlueprint> substitute(const type_substitution_map& sub_map);

            /** replace with resolved type description.
             *  Promise:
             *  1. ref().td()  == @p td
             *  2. this->is_concrete() == true
             *  3. this->is_variable() == false
             **/
            void resolve_to(TypeDescr td);

            /** write human-readable representation to stream @p os **/
            void display(std::ostream & os) const;

        private:
            /** construct blueprint for @p ref **/
            explicit TypeBlueprint(const type_ref & ref);

        private:
            /** name of the type being constrained here **/
            type_ref ref_;

            // additional descriptive info..
        };

        inline std::ostream &
        operator<<(std::ostream & os, const TypeBlueprint & x) {
            x.display(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/


/** end TypeBlueprint.hpp **/
