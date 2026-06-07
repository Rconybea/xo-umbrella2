/** @file type_ref.hpp **/

#pragma once

#include "xo/flatstring/flatstring.hpp"
#include "xo/reflect/TypeDescr.hpp"

namespace xo {
    namespace scm {
        using prefix_type = xo::flatstring<8>;
        using type_var = xo::flatstring<20>;

        /** @class type_ref
         *  @brief name and eventual resolution for type associated with an expression.
         *
         *  Type inference / unification operates on
         *  @ref xo::scm::TypeBlueprint instances, see also.
         **/
        class type_ref {
        public:
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            type_ref() = default;
            type_ref(const type_var& id, TypeDescr td);

            /** if type not determined (@p td is nullptr),
             *  -> generate and store type variable name.
             *  otherwise type already resolvedn
             **/
            static type_ref dwim(prefix_type prefix, TypeDescr td);

            /** generate a unique type-variable name, that begins with @p prefix **/
            static type_var generate_unique(prefix_type prefix);

            const type_var& id() const { return id_; }
            TypeDescr td() const { return td_; }

            /** true iff type at this location has been resolved **/
            bool is_concrete() const;

            void resolve_to(TypeDescr td);

        private:
            /** unique (likely generated) name for type at this location **/
            type_var id_;
            /** description for concrete type, once resolved.
             *  may be null when type_ref created.
             *  expected to be immutable once established.
             *  note that TypeDescr itself may be incomplete,
             *  but not for inference purposes
             **/
            TypeDescr td_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/** end type_ref.hpp **/
