/* file JsonPrinter.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "PrintJson.hpp"
//#include "time/Time.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <cmath>

namespace xo {
    using xo::time::utc_nanos;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::SelfTagging;
    using xo::reflect::TypeDescr;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TaggedRcptr;
    using xo::print::quot;
    using xo::time::iso8601;
    using xo::xtag;

    namespace json {
        TaggedRcptr
        PrintJson::self_tp()
        {
            return Reflect::make_rctp(this);
            //return TaggedRcptr::make(this);
        } /*self_tp*/

        void
        JsonPrinter::report_internal_type_consistency_error(TypeDescr td1,
                                                            TypeDescr td2,
                                                            std::ostream * p_os) const
        {
            *p_os << "<internal-error: type mismatch between T & S"
                  << xtag("T", td1->canonical_name())
                  << xtag("S", td2->canonical_name())
                  << ">";
        } /*report_internal_type_consistency_error*/

        namespace {
            /* this will be used when TaggedPtr refers to a pointer-like value,
             * e.g.
             *    xo::ref::rp<T>
             */
            void
            print_generic_pointer(PrintJson const & print_json,
                                  TaggedPtr tp,
                                  std::ostream * p_os)
            {
                /* e.g. if
                 *   xo::ref::rp<VanillaOption> opt = ...;
                 * then expect to print just as we would for
                 *   VanillaOption & opt = ...;
                 * if pointer is null,  will print {}
                 */

                if (tp.n_child()) {
                    print_json.print_aux(tp.get_child(0), p_os);
                } else {
                    /* note: this can be distinguished from a bona fide struct,
                     * b/c it doesn't supply the _name_ member
                     */
                    *p_os << "{}";
                }
            } /*print_generic_pointer*/

            /* this will be used when TaggedPtr refers to a vector-like value,
             * e.g.
             *    std::vector<T>
             *    std::array<T, N>
             */
            void
            print_generic_vector(PrintJson const & print_json,
                                 TaggedPtr tp,
                                 std::ostream * p_os)
            {
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

                    print_json.print_aux(tp.get_child(i), p_os);
                }

                *p_os << "]";
            } /*print_generic_vector*/

            /* this will be used when TaggedPtr is understood to refer to a struct-like value.
             */
            void
            print_generic_struct(PrintJson const & print_json,
                                 TaggedPtr tp,
                                 std::ostream * p_os)
            {
                /* e.g. if
                 *   struct Foo { int x_; double y_; };
                 *   Foo foo{1, 1.4142};
                 *
                 * then expect to print
                 *   {"_name_": "Foo", "x": 1, "y": 1.4142}
                 *
                 * note that python json parser requires property names in double quotes
                 */

                *p_os << "{";

                *p_os << "\"_name_\": \"" << tp.td()->short_name() << "\"";

                for (uint32_t i = 0, n = tp.n_child(); i < n; ++i) {
                    *p_os << ", \"" << tp.struct_member_name(i) << "\": ";

                    print_json.print_aux(tp.get_child(i), p_os);
                }

                *p_os << "}";
            } /*print_generic_struct*/

        } /*namespace*/

        void
        PrintJson::print_aux(TaggedPtr tp,
                             std::ostream * p_os) const
        {
            if (tp.td()) {
                TypeId id = tp.td()->id();

                std::unique_ptr<JsonPrinter> const * printer
                    = this->printer_map_.lookup(id);

                if (printer && *printer) {
                    (*printer)->print_json(tp, p_os);
                } else {
                    /* if no special-case printer,  apply generic printing behavior */
                    switch (tp.td()->metatype()) {
                    case Metatype::mt_pointer:
                        print_generic_pointer(*this, tp, p_os);
                        return;
                    case Metatype::mt_vector:
                        print_generic_vector(*this, tp, p_os);
                        return;
                    case Metatype::mt_struct:
                        print_generic_struct(*this, tp, p_os);
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
        } /*print_aux*/

        void
        PrintJson::print_tp(TaggedPtr tp,
                            std::ostream * p_os) const
        {
            this->print_aux(tp, p_os);
            //*p_os << std::ends;
        } /*print*/

        void
        PrintJson::print_obj(rp<SelfTagging> const & obj, std::ostream * p_os) const
        {
            assert(obj.get());

            this->print_tp(obj->self_tp(), p_os);
        } /*print_obj*/

        /* Consider:
         *   TaggedPtr tp = ...;
         *   std::ostream * p_os = ...;
         *
         *   PrintJson * pjson = PrintJsonSingleton::instance();
         *
         *   // print json representation,  depending on runtime type of tp's target
         *   pjson->print_tp(tp, p_os);
         *
         *   // can also use .print(),  relying on JsonPrinter_TaggedPtr
         *   // .print() will next original TaggedPtr in another;
         *   // this shim unwinds that
         *   //
         *   pjson->print(tp, p_os);
         */
        class JsonPrinter_TaggedPtr : public JsonPrinter {
        public:
            JsonPrinter_TaggedPtr(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                TaggedPtr * x = this->check_recover_native<TaggedPtr>(tp, p_os);

                if (x) {
                    this->pjson()->print_tp(*x, p_os);
                }
            } /*print_json*/
        }; /*JsonPrinter_TaggedPtr*/

        namespace {
            void
            provide_tagged_ptr_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_TaggedPtr(p_json));

                p_json->provide_printer(Reflect::require<TaggedPtr>(),
                                        std::move(printer));
            } /*provide_tagged_ptr_printer*/
        } /*namespace*/

        class JsonPrinter_bool : public JsonPrinter {
        public:
            JsonPrinter_bool(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                bool * x = this->check_recover_native<bool>(tp, p_os);

                if (x) {
                    /* json boolean format is lower case true/false.
                     * (note that this conflicts with python True/False,  achtung!)
                     */
                    *p_os << (*x ? "true" : "false");
                }
            } /*print_json*/
        }; /*JsonPrinter_bool*/

        namespace {
            void
            provide_bool_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_bool(p_json));

                p_json->provide_printer(Reflect::require<bool>(),
                                        std::move(printer));
            } /*provide_bool_printer*/
        } /*namespace*/

        template<typename T>
        class JsonPrinter_integer : public JsonPrinter {
        public:
            JsonPrinter_integer(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                T * x = tp.recover_native<T>();

                if (x) {
                    *p_os << *x;
                } else {
                    report_internal_type_consistency_error(Reflect::require<T>(),
                                                           tp.td(),
                                                           p_os);
                }
            } /*print_json*/
        }; /*JsonPrinter_integer*/

        namespace {
            template<typename T>
            void
            provide_integer_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_integer<T>(p_json));

                p_json->provide_printer(Reflect::require<T>(), std::move(printer));
            } /*provide_integer_printer*/
        }

        template<typename T>
        class JsonPrinter_floatingpoint : public JsonPrinter {
        public:
            JsonPrinter_floatingpoint(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override
                {
                    T * x = tp.recover_native<T>();

                    if (x) {
                        if (std::isfinite(*x)) {
                            *p_os << *x;
                        } else {
                            /* special cases.
                             * use javascript-friendly format
                             *
                             * Note non-finite floating-point values are not representable in
                             *      standard json (?!#),  though it's a standard extension
                             */

                            if (std::isnan(*x))
                                *p_os << "NaN";
                            else if (*x > 0.0)
                                *p_os << "Infinity";
                            else
                                *p_os << "-Infinity";
                        }
                    } else {
                        report_internal_type_consistency_error(Reflect::require<T>(),
                                                               tp.td(),
                                                               p_os);
                    }
                } /*print_json*/
        }; /*JsonPrinter_floatingpoint*/

        namespace {
            template<typename T>
            void
            provide_floatingpoint_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_floatingpoint<T>(p_json));

                p_json->provide_printer(Reflect::require<T>(), std::move(printer));
            } /*provide_floatingpoint_printer*/
        } /*namespace*/

        template<typename T>
        class JsonPrinter_string : public JsonPrinter {
        public:
            JsonPrinter_string(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                T * x = tp.recover_native<T>();

                if (x) {
                    /* TODO: escapes special characters */
                    *p_os << quot(*x);
                } else {
                    report_internal_type_consistency_error(Reflect::require<T>(),
                                                           tp.td(),
                                                           p_os);
                }
            } /*print_json*/
        }; /*JsonPrinter_string*/

        namespace {
            template<typename T>
            void
            provide_string_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_string<T>(p_json));

                p_json->provide_printer(Reflect::require<T>(), std::move(printer));
            } /*provide_string_printer*/
        } /*namespace */

        class JsonPrinter_utc_nanos : public JsonPrinter {
        public:
            JsonPrinter_utc_nanos(PrintJson * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                utc_nanos * x = tp.recover_native<utc_nanos>();

                if (x) {
                    /* format like
                     *   "2012-04-23T18:25:43.511Z"
                     * since that's what javascript uses
                     */
                    *p_os << "\"" << iso8601(*x) << "\"";
                } else {
                    report_internal_type_consistency_error(Reflect::require<utc_nanos>(),
                                                           tp.td(),
                                                           p_os);
                }
            } /*print_json*/
        }; /*JsonPrinter_utc_nanos*/

        namespace {
            void
            provide_utc_nanos_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_utc_nanos(p_json));

                p_json->provide_printer(Reflect::require<utc_nanos>(),
                                        std::move(printer));
            } /*provide_utc_nanos_printer*/
        } /*namespace*/

        PrintJson::PrintJson() {
            this->provide_std_printers();
        } /*ctor*/

        /* provide printers for common basic types */
        void
        PrintJson::provide_std_printers()
        {
            provide_tagged_ptr_printer(this);

            provide_bool_printer(this);

            provide_integer_printer<std::int16_t>(this);
            provide_integer_printer<std::uint16_t>(this);
            provide_integer_printer<std::int32_t>(this);
            provide_integer_printer<std::uint32_t>(this);
            provide_integer_printer<std::int64_t>(this);
            provide_integer_printer<std::uint64_t>(this);

            provide_floatingpoint_printer<float>(this);
            provide_floatingpoint_printer<double>(this);

            provide_string_printer<char *>(this);
            provide_string_printer<char const *>(this);
            provide_string_printer<std::string>(this);
            provide_string_printer<std::string_view>(this);

            provide_utc_nanos_printer(this);
        } /*provide_std_printers*/

        rp<PrintJson>
        PrintJsonSingleton::s_instance;

        rp<PrintJson>
        PrintJsonSingleton::instance()
        {
            if (!s_instance)
                s_instance = new PrintJson();

            return s_instance;
        } /*instance*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonPrinter.cpp */
