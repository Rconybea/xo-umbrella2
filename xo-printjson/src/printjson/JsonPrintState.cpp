/** @file JsonPrintState.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonPrintState.hpp"
#include "JsonObject.hpp"
#include "PrintJson.hpp"
#include "type_keys.hpp"
#include <xo/reflect/enum/EnumTdx.hpp>
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/arena/backtrace.hpp>
#include <xo/ppsink/quoted_ostream.hpp>     /* os << quot(..) */
#include <xo/ppsink/tag_ostream.hpp>        /* os << xtag(..) */
#include <cstdlib>
#include <iostream>

namespace xo {
    using xo::reflect::EnumTdx;
    using xo::reflect::Metatype;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::TypeId;

    namespace json {
        /* one scope in from namespace xo: see PrintJson.cpp */
        using xo::pp::quot;
        using xo::pp::tostr;
        using xo::pp::xtag;

        JsonPrintState::JsonPrintState(PrintJson const * pjson, std::ostream * p_os)
            : pjson_{pjson}, p_os_{p_os}, max_depth_{pjson->max_depth()}
        {}

        void
        JsonPrintState::abort_too_deep(TaggedPtr tp) const
        {
            std::string why
                = tostr("PrintJson: nesting would exceed max_depth (",
                        this->max_depth_,
                        ") printing ",
                        (tp.td() ? tp.td()->canonical_name() : std::string("<null td>")),
                        ". Graph nested deeper than this limit, or a cycle through"
                        " values printed without identity (JsonPrintState::print_value,"
                        " or an object sharing its parent's address)");

            std::cerr << why << std::endl;
            print_backtrace(true /*demangle_flag*/);
            /* again, below what may be thousands of frames */
            std::cerr << why << std::endl;
            std::abort();
        } /*abort_too_deep*/

        void
        JsonPrintState::abort_misuse(char const * what) const
        {
            std::string why = tostr("PrintJson: json printer misuse: ", what);

            std::cerr << why << std::endl;
            print_backtrace(true /*demangle_flag*/);
            std::cerr << why << std::endl;
            std::abort();
        } /*abort_misuse*/

        bool
        JsonPrintState::has_printer(TypeDescr td) const
        {
            return this->pjson_->has_printer(td);
        } /*has_printer*/

        namespace {
            /** one print() call in progress, for its lifetime: also when a
             *  printer throws (see PrintJson::validate_tp)
             **/
            class DepthScope {
            public:
                explicit DepthScope(std::uint32_t * p_depth) : p_depth_{p_depth} { ++*p_depth_; }
                ~DepthScope() { --*p_depth_; }

                DepthScope(DepthScope const &) = delete;
                DepthScope & operator=(DepthScope const &) = delete;

            private:
                std::uint32_t * p_depth_;
            };

            /** restores *@p p_slot on exit -- also when a printer throws **/
            template <typename T>
            class RestoreScope {
            public:
                explicit RestoreScope(T * p_slot) : p_slot_{p_slot}, saved_{*p_slot} {}
                ~RestoreScope() { *p_slot_ = saved_; }

                RestoreScope(RestoreScope const &) = delete;
                RestoreScope & operator=(RestoreScope const &) = delete;

            private:
                T * p_slot_;
                T saved_;
            };

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
                /* e.g. if
                 *   struct Foo { int x_; double y_; };   // REFLECT_MEMBER(sr, x), (sr, y)
                 *   Foo foo{1, 1.4142};
                 *
                 * then expect to print
                 *   {"_name_": "Foo", "_canonical_type_": "xo::Foo", "_short_type_": "Foo",
                 *    "_id_": 1,
                 *    "_members_": [{"_name_": "x", "_canonical_type_": "int", "_short_type_": "int",
                 *                   "_metatype_": "atomic", "_value_": 1},
                 *                  {"_name_": "y", .., "_value_": 1.4142}]}
                 *
                 * Members-style, as JsonMembers writes them, under their
                 * reflected names: so a member may be called _id_ or _ref_
                 * without ambiguity.  Until 2026-10-05 members were top-level
                 * keys ("x": 1) -- .xo-backlog/xo-printjson/issues/07.
                 *
                 * see type_keys
                 *
                 * note that python json parser requires property names in double quotes
                 */
                JsonObject obj = state.open_object(tp);

                obj.members()
                    .reflected_members(tp)
                    .end();

                obj.close();
            } /*print_generic_struct*/

            /* a reflected enum (EnumReflector): its enumerator's name, a
             * json string; or, a value no enumerator has, its integer, a
             * json number -- so a consumer can tell them apart
             * (.xo-backlog/xo-reflect/issues/06)
             */
            void
            print_reflected_enum(EnumTdx const & ei,
                                 TaggedPtr tp,
                                 std::ostream * p_os)
            {
                if (std::string const * name = ei.name_of(tp.address()))
                    *p_os << quot(*name);
                else
                    *p_os << ei.value_of(tp.address());
            } /*print_reflected_enum*/

        } /*namespace*/

        void
        JsonPrintState::print(TaggedPtr tp)
        {
            this->print_node(tp, true /*identity*/);
        } /*print*/

        void
        JsonPrintState::print_value(TaggedPtr tp)
        {
            this->print_node(tp, false /*!identity*/);
        } /*print_value*/

        JsonPrintState::ObjectEntry &
        JsonPrintState::entry_for(void const * p)
        {
            auto ix = this->objects_.try_emplace(p, ObjectEntry{this->next_id_, nullptr});

            if (ix.second)
                ++(this->next_id_);

            return ix.first->second;
        } /*entry_for*/

        void
        JsonPrintState::print_ref(void const * p)
        {
            if (!p) {
                *p_os_ << "null";
                return;
            }

            *p_os_ << "{" << quot("_ref_") << ": " << this->entry_for(p).id_ << "}";
        } /*print_ref*/

        JsonObject
        JsonPrintState::open_object(std::string_view name, TypeDescr td)
        {
            if (!this->pending_.open_)
                this->abort_misuse("open_object() twice in one print_json, or outside print_json");

            this->pending_.open_ = false;

            /* the value print_node dispatched: owns its entry under the
             * type it printed as (pending_.type_), whatever td names
             */
            JsonObject retval = this->open_object_aux(this->pending_.address_, name, td);

            if (this->pending_.address_)
                this->objects_[this->pending_.address_].type_ = this->pending_.type_;

            return retval;
        } /*open_object*/

        bool
        JsonPrintState::is_printed(void const * p) const
        {
            auto ix = this->objects_.find(p);

            return (ix != this->objects_.end()) && (ix->second.type_ != nullptr);
        } /*is_printed*/

        JsonObject
        JsonPrintState::open_object_at(void const * p, std::string_view name, TypeDescr td)
        {
            if (this->is_printed(p))
                this->abort_misuse("open_object_at() for an object printed already (ask is_printed())");

            return this->open_object_aux(p, name, td);
        } /*open_object_at*/

        JsonObject
        JsonPrintState::open_object_aux(void const * p, std::string_view name, TypeDescr td)
        {
            *p_os_ << "{" << quot("_name_") << ": " << quot(name)
                   << ", " << json::type_keys(td);

            if (p) {
                /* first object printed at this address: it owns the entry,
                 * claiming one a ref made earlier
                 */
                ObjectEntry & e = this->entry_for(p);

                e.type_ = td;
                *p_os_ << ", " << quot("_id_") << ": " << e.id_;
            }

            return JsonObject(this);
        } /*open_object_aux*/

        JsonObject
        JsonPrintState::open_object(TaggedPtr tp)
        {
            return this->open_object(tp.td()->short_name(), tp.td());
        } /*open_object*/

        void
        JsonPrintState::print_node(TaggedPtr tp, bool identity)
        {
            if (this->depth_ >= this->max_depth_)
                this->abort_too_deep(tp);

            DepthScope scope(&this->depth_);
            std::ostream * p_os = this->p_os_;

            if (tp.td()) {
                TypeId id = tp.td()->id();

                JsonPrinter const * printer = this->pjson_->lookup_printer(id);

                /* identity applies to values printed as json objects */
                bool is_object = (printer
                                  ? printer->prints_object()
                                  : tp.td()->metatype() == Metatype::mt_struct);
                void const * address = nullptr;

                if (identity && is_object && tp.address()) {
                    auto ix = this->objects_.find(tp.address());

                    if (ix == this->objects_.end() || ix->second.type_ == nullptr) {
                        /* first time printed (perhaps referred to already) */
                        address = tp.address();
                    } else if (ix->second.type_ == tp.td()) {
                        /* printed already: refer to it */
                        *p_os << "{" << quot("_ref_") << ": " << ix->second.id_ << "}";
                        return;
                    } else {
                        /* another type at a printed object's address: part
                         * of that object (its first member, say).  In full,
                         * without identity
                         */
                    }
                }

                RestoreScope<Pending> restore(&this->pending_);
                this->pending_ = Pending{true, address, tp.td()};

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
                    case Metatype::mt_atomic:
                        if (EnumTdx const * ei = tp.td()->enum_info()) {
                            print_reflected_enum(*ei, tp, p_os);
                            return;
                        }
                        break;
                    case Metatype::mt_invalid:
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
        } /*print_node*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonPrintState.cpp */
