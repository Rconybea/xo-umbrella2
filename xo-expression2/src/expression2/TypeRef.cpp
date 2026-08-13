/** @file TypeRef.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "TypeRef.hpp"
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/reflect/TypeDescr_pp.hpp>   /* Prettifier<TypeDescr> for the :td field */
#include <xo/facet/FacetRegistry.hpp>
#include <xo/ppsink/pretty_struct.hpp>   /* sink.struct_open(..) */
#include <xo/ppsink/quoted.hpp>          /* xo::pp::quot */
#include <string_view>

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;

    namespace scm {
        TypeRef::TypeRef(const type_var & id, obj<AType> type)
                : id_{id}, type_{type}
        {}

        TypeRef::TypeRef(const type_var & id, TypeDescr td)
                : id_{id}, td_{td}
        {}

        TypeRef
        TypeRef::resolved(obj<AType> type)
        {
            assert(type);

            type_var null;
            return TypeRef(null, type);
        }

        TypeRef
        TypeRef::resolved(TypeDescr td)
        {
            assert(td);

            type_var null;
            return TypeRef(null, td);
        }

        TypeRef
        TypeRef::dwim(prefix_type prefix, obj<AType> type)
        {
            if (type) {
                /* type already resolved
                 *  -> we don't need a type variable name
                 */
                return TypeRef::resolved(type);
            } else {
                /* type is not resolved yet.
                 *  -> give it a unique name,
                 *     to seed unification
                 */
                return TypeRef(generate_unique(prefix), type);
            }
        }

        TypeRef
        TypeRef::dwim(prefix_type prefix, TypeDescr td)
        {
            if (td) {
                /* type already resolved
                 *  -> we don't need a type variable name
                 */
                return TypeRef::resolved(td);
            } else {
                /* type is not resolved yet.
                 *  -> give it a unique name,
                 *     to seed unification
                 */
                return TypeRef(generate_unique(prefix), td);
            }
        }

        auto
        TypeRef::generate_unique(prefix_type prefix) -> type_var
        {
            static uint32_t s_counter = 0;

            s_counter = (1 + s_counter) % 1000000000;

            char buf[type_var::fixed_capacity];
            int n = snprintf(buf, sizeof(buf), "%s:%u", prefix.c_str(), s_counter);
            (void)n;;

            assert(n < static_cast<int>(type_var::fixed_capacity));

            // not necessary, but to remove all doubt.
            buf [sizeof(buf) - 1] = '\0';

            return type_var(buf);
        }

        bool
        TypeRef::is_resolved() const noexcept
        {
            return (td_ != nullptr);
        }

        void
        TypeRef::visit_gco_children(VisitReason reason,
                                    obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_poly_child(reason, &type_);
        }

        void
        TypeRef::pretty(xo::pp::PpSink & sink) const
        {
            /* named locals: field() captures BY REFERENCE (pretty_struct.hpp) */
            const auto id = xo::pp::quot(id_);

            auto st = sink.struct_open("TypeRef");

            st.field("id", id);

            /* Prettifier<TypeDescr> renders NOTHING for a null descriptor
             * (TypeDescr_pp.hpp), which for an unresolved TypeRef -- the normal
             * pre-typecheck state -- would leave a bare ":td" with no value.
             * Legacy said "null" here (cond(td_, td_, "null")); keep that,
             * rather than change xo-reflect's policy from this side.
             */
            if (td_) {
                st.field("td", td_);
            } else {
                static constexpr std::string_view c_null = "null";

                st.field("td", c_null);
            }
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end TypeRef.cpp */
