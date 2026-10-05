/** @file JsonPrintState.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonPrintState.hpp"
#include "PrintJson.hpp"
#include "type_keys.hpp"
#include <xo/ppsink/tag_ostream.hpp>        /* os << xtag(..) */

namespace xo {
    using xo::reflect::Metatype;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::TypeId;

    namespace json {
        /* one scope in from namespace xo: see PrintJson.cpp */
        using xo::pp::xtag;

        JsonPrintState::JsonPrintState(PrintJson const * pjson, std::ostream * p_os)
            : pjson_{pjson}, p_os_{p_os}
        {}

        bool
        JsonPrintState::has_printer(TypeDescr td) const
        {
            return this->pjson_->has_printer(td);
        } /*has_printer*/

        namespace {
            /* this will be used when TaggedPtr refers to a pointer-like value,
             * e.g.
             *    xo::ref::rp<T>
             */
            void
            print_generic_pointer(JsonPrintState & state,
                                  TaggedPtr tp)
            {
                std::ostream * p_os = state.p_os();

                /* e.g. if
                 *   xo::ref::rp<VanillaOption> opt = ...;
                 * then expect to print just as we would for
                 *   VanillaOption & opt = ...;
                 * if pointer is null,  will print {}
                 */

                if (tp.n_child()) {
                    state.print(tp.get_child(0));
                } else {
                    /* was "{}" until 2026-09-21, distinguishable from a real
                     * struct only by the absent _name_ member.  json null says
                     * the same thing without asking a consumer to notice an
                     * absence, and it is what the bespoke pointer printers
                     * (JsonPrinter_ObjectSlot, JsonPrinter_RootSet) already
                     * emit -- so routing a pointer through this path is no
                     * longer a change in what a null looks like.
                     */
                    *p_os << "null";
                }
            } /*print_generic_pointer*/

            /* this will be used when TaggedPtr refers to a vector-like value,
             * e.g.
             *    std::vector<T>
             *    std::array<T, N>
             */
            void
            print_generic_vector(JsonPrintState & state,
                                 TaggedPtr tp)
            {
                std::ostream * p_os = state.p_os();

                /* e.g. if
                 *   std::array<double, 3> v{1, 2, 3};
                 *
                 * then expect to print
                 *   [1.0, 2.0, 3.0]
                 */

                *p_os << "[";

                for (uint32_t i = 0, n = tp.n_child(); i < n; ++i) {
                    if (i > 0)
                        *p_os << ", ";

                    state.print(tp.get_child(i));
                }

                *p_os << "]";
            } /*print_generic_vector*/

            /* this will be used when TaggedPtr is understood to refer to a struct-like value.
             */
            void
            print_generic_struct(JsonPrintState & state,
                                 TaggedPtr tp)
            {
                std::ostream * p_os = state.p_os();

                /* e.g. if
                 *   struct Foo { int x_; double y_; };
                 *   Foo foo{1, 1.4142};
                 *
                 * then expect to print
                 *   {"_name_": "Foo", "_canonical_type_": "xo::Foo", "_short_type_": "Foo",
                 *    "x": 1, "y": 1.4142}
                 *
                 * see type_keys
                 *
                 * note that python json parser requires property names in double quotes
                 */

                *p_os << "{";

                *p_os << "\"_name_\": \"" << tp.td()->short_name() << "\""
                      << ", " << json::type_keys(tp.td());

                for (uint32_t i = 0, n = tp.n_child(); i < n; ++i) {
                    *p_os << ", \"" << tp.struct_member_name(i) << "\": ";

                    state.print(tp.get_child(i));
                }

                *p_os << "}";
            } /*print_generic_struct*/

        } /*namespace*/

        void
        JsonPrintState::print(TaggedPtr tp)
        {
            std::ostream * p_os = this->p_os_;

            if (tp.td()) {
                TypeId id = tp.td()->id();

                JsonPrinter const * printer = this->pjson_->lookup_printer(id);

                if (printer) {
                    printer->print_json(tp, *this);
                } else {
                    /* if no special-case printer,  apply generic printing behavior */
                    switch (tp.td()->metatype()) {
                    case Metatype::mt_pointer:
                        print_generic_pointer(*this, tp);
                        return;
                    case Metatype::mt_vector:
                        print_generic_vector(*this, tp);
                        return;
                    case Metatype::mt_struct:
                        print_generic_struct(*this, tp);
                        return;
                    case Metatype::mt_function:
                        /** new branch (added for xo-expression / xo-jit) **/
                        (*p_os) << "<error-json-printer-not-implemented"
                                << xtag("type", tp.td()->canonical_name())
                                << xtag("metatype", tp.td()->metatype())
                                << ">";
                        return;
                    case Metatype::mt_invalid:
                    case Metatype::mt_atomic:
                        break;
                    }

                    (*p_os) << "<error-json-printer-not-found"
                            << xtag("type", tp.td()->canonical_name())
                            << xtag("metatype", tp.td()->metatype())
                            << ">";
                }
            } else {
                (*p_os) << "<error-null-tp>";
            }
        } /*print*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonPrintState.cpp */
