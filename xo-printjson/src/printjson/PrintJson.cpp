/* file JsonPrinter.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "PrintJson.hpp"
#include <xo/reflect/TypeDescr.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/arena/MemorySizeInfo.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/facet/handlestore/ObjectSlot.hpp>
#include <xo/facet/handlestore/DHandleStore.hpp>
#include <xo/facet/TypeRegistry.hpp>
#include <xo/arena/DArena.hpp>
#include <xo/flatstring/flatstring.hpp>     /* os << quot(..) */
#include <xo/ppsink/quoted_ostream.hpp>
#include <xo/ppsink/tag_ostream.hpp>        /* os << xtag(..) */
#include <xo/ppsink/pp_time_ostream.hpp>    /* os << iso8601(..) */
#include <xo/timeutil/timeutil.hpp>
#include <cmath>
#include <type_traits>

namespace xo {
    using xo::time::utc_nanos;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::SelfTagging;
    using xo::reflect::TypeDescr;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TaggedRcptr;

    namespace json {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
        using xo::pp::iso8601;
        using xo::pp::quot;
        using xo::pp::xtag;

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

        void
        PrintJson::validate_tp(TaggedPtr tp) const
        {
            /* the throw comes from n_child()/get_child(), which is where
             * xo-reflectable2 rotates an erased fop to AReflectable.  Nothing
             * to do per node, so the visitor is empty: this is a reuse of
             * reflect's walker, not a second traversal implementation.
             */
            TaggedPtr::visit_tree_preorder(tp, [](TaggedPtr) {});
        } /*validate_tp*/

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
                    if constexpr (std::is_pointer_v<T>) {
                        /* a null char pointer.  Rendering it as json null
                         * rather than "" keeps it distinguishable from an
                         * empty string, and -- the reason this branch exists
                         * -- quot(nullptr) SEGFAULTS.  Measured 2026-09-21;
                         * both char pointer types have been registered here
                         * since before that, so this was live.
                         *
                         * Reachable because raw pointers are reflected as of
                         * .xo-backlog/xo-reflect/issues/01, which made it
                         * worth fixing rather than noting.
                         */
                        if (*x == nullptr) {
                            *p_os << "null";
                            return;
                        }
                    }

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

        /** flatstring<N> renders as a json string.
         *
         *  Separate from JsonPrinter_string because the conversion has to be
         *  explicit: flatstring has BOTH operator std::string_view() and
         *  operator const char*(), so quot(*x) is ambiguous.
         **/
        template<std::size_t N>
        class JsonPrinter_flatstring : public JsonPrinter {
        public:
            JsonPrinter_flatstring(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                xo::flatstring<N> * x = tp.recover_native<xo::flatstring<N>>();

                if (x) {
                    *p_os << quot(std::string_view(*x));
                } else {
                    report_internal_type_consistency_error(Reflect::require<xo::flatstring<N>>(),
                                                           tp.td(),
                                                           p_os);
                }
            } /*print_json*/
        }; /*JsonPrinter_flatstring*/

        namespace {
            template<std::size_t N>
            void
            provide_flatstring_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_flatstring<N>(p_json));

                p_json->provide_printer(Reflect::require<xo::flatstring<N>>(), std::move(printer));
            } /*provide_flatstring_printer*/
        } /*namespace*/

        /** an address renders as a DECIMAL integer, not hex and not a string.
         *
         *  json has no hex literal (RFC 8259 section 6), so 0x... would have to
         *  be a string and cost every consumer a parseInt.  Decimal parses
         *  straight to a number.
         *
         *  NB javascript numbers are IEEE754 doubles, so this is exact only
         *  below 2^53.  A 48-bit address has ~68x headroom; a 57-bit one (5-level
         *  paging) does not, and the rounding is undetectable on the consumer's
         *  side.  Prefer reporting an OFFSET into a known region where the value
         *  matters -- see JsonPrinter_ObjectSlot's `offset' below.
         **/
        class JsonPrinter_address : public JsonPrinter {
        public:
            JsonPrinter_address(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                const void ** x = tp.recover_native<const void *>();

                if (x) {
                    *p_os << reinterpret_cast<std::uintptr_t>(*x);
                } else {
                    report_internal_type_consistency_error(Reflect::require<const void *>(),
                                                           tp.td(),
                                                           p_os);
                }
            } /*print_json*/
        }; /*JsonPrinter_address*/

        namespace {
            void
            provide_address_printer(PrintJson * p_json)
            {
                std::unique_ptr<JsonPrinter> printer(new JsonPrinter_address(p_json));

                p_json->provide_printer(Reflect::require<const void *>(), std::move(printer));
            } /*provide_address_printer*/
        } /*namespace*/

        /** @brief json printer for xo::facet::ObjectSlot
         *
         *  xo::facet::ObjectSlot represents a root object in
         *  xo::facet::AllocFlywheel.
         *
         *  We want a bespoke printer so that we can print
         *  the root object's offset relative to the start of its storage.
         *
         *  This is valid for AllocFlywheel instances,
         *  since they're guaranteed to be hosted by an Arena
         *  with storage aligned on @ref DHandleStoreBase::storage_base_align()
         *
         *  This isn't true of obj<ATop> in general, so the separate
         *  ObjectSlot type is necessary.
         *
         *  - @c typeseq / @c type: from ATop, no rotation.  @c type reads the
         *    sentinel name for a type never registered with TypeRegistry --
         *    which is every type, seen from a pybind module, until
         *    .xo-backlog/xo-facet/issues/01 lands.
         *  - @c offset: bytes from the base of the arena that owns the object.
         *    Recovered from the object pointer ALONE via DArena::obj2arena,
         *    which is what the maskable base alignment exists for
         *    (.xo-backlog/xo-arena/issues/04).  An offset rather than an
         *    address because json numbers are IEEE754 doubles on the consumer's
         *    side; see JsonPrinter_address.
         *  - @c size: bytes the allocation occupies, from the arena's alloc
         *    header -- @c AllocInfo::size(), which includes allocator padding
         *    and excludes the header itself.  NOT @c sizeof(DRepr): DArray and
         *    DString fix capacity at construction rather than in the type, so
         *    the representation size says nothing about the payload and a cell
         *    drawn from it would be wrong by an unbounded factor.
         *
         *  Both of those calls are sound because a non-empty ObjectSlot can
         *  only be made by a DHandleStore, which checked the pointer against
         *  its own arena and checked that arena's headers and alignment.  See
         *  ObjectSlot's Provenance note and .xo-backlog/xo-facet/issues/04.
         *
         *  An EMPTY slot renders as @c null.  Slots are emitted including the
         *  empty ones, so a consumer reads a slot's index from its position in
         *  the enclosing array.
         **/
        class JsonPrinter_ObjectSlot : public JsonPrinter {
        public:
            JsonPrinter_ObjectSlot(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                using xo::facet::ObjectSlot;
                using xo::facet::DHandleStoreBase;
                using xo::facet::TypeRegistry;
                using xo::mm::DArena;

                ObjectSlot * x = tp.recover_native<ObjectSlot>();

                if (!x) {
                    report_internal_type_consistency_error(Reflect::require<ObjectSlot>(),
                                                           tp.td(),
                                                           p_os);
                    return;
                }

                void * data = x->opaque_data();

                if (!data) {
                    /* a released slot.  Emitted rather than skipped, so array
                     * position remains the slot index
                     */
                    *p_os << "null";
                    return;
                }

                auto tseq = x->_typeseq();

                *p_os << "{"
                      << "\"_name_\": " << quot("ObjectSlot")
                      << ", \"typeseq\": " << tseq.seqno()
                      << ", \"type\": " << quot(TypeRegistry::id2name(tseq));

                /* 0 means no FacetAppcx has been constructed, so there is no
                 * agreed alignment to mask with.  Reporting the absence beats
                 * masking with ~(0-1) == 0 and dereferencing the result.
                 *
                 * NOT REACHABLE for a slot that came from a flywheel, and
                 * deliberately kept anyway.  A non-empty slot implies a
                 * DHandleStore, whose ctor rejects a zero base alignment, and
                 * assign_storage_base_align is write-once -- so the value
                 * cannot fall back to 0 underneath a live slot.  The guard
                 * survives because `data' arrives through recover_native from
                 * a TaggedPtr a caller assembled, and the failure it prevents
                 * is a segfault rather than a wrong number.
                 */
                std::size_t align_z = DHandleStoreBase::storage_base_align();

                if (align_z == 0) {
                    *p_os << ", \"offset\": null, \"size\": null}";
                } else {
                    DArena * arena = DArena::obj2arena(data, align_z);

                    *p_os << ", \"offset\": "
                          << (static_cast<const std::byte *>(data) - arena->_mem_lo())
                          << ", \"size\": "
                          << arena->alloc_info(static_cast<std::byte *>(data)).size()
                          << "}";
                }
            } /*print_json*/
        }; /*JsonPrinter_ObjectSlot*/


        /** @brief json printer for a flywheel's strong root set
         *
         *  Keyed on the POINTEE, @c DHandleArena<ObjectSlot>.
         *
         *  It was keyed on the POINTER until 2026-09-21, doing its own
         *  dereference, because a raw pointer had no @c EstablishTdx
         *  specialisation and so reflected as an atom -- never reaching
         *  @c print_generic_pointer's dispatch to a child.  Raw pointers are
         *  reflected now (.xo-backlog/xo-reflect/issues/01), which is what
         *  let the key move to the pointee.
         *
         *  Its caller is @c JsonPrinter_AllocFlywheel, which reaches the store
         *  DIRECTLY via @c AllocFlywheel::strong_root_set -- there is no
         *  pointer in the path any more, since @c FlywheelInfo went the day
         *  after.  The pointee key is still the right one; it is simply no
         *  longer the flywheel frame that exercises raw-pointer reflection.
         *
         *  This printer is what retired @c RootSetInfo on 2026-09-21.  That
         *  struct held size/capacity/live/free/slots, copied out of the store
         *  by @c DHandleStore::snapshot and described a third time by a
         *  StructReflector -- one fact in three places, in a tree where that
         *  shape has gone wrong repeatedly.  The same five fields are now read
         *  straight off the live store.
         *
         *  The wire schema is UNCHANGED by that move, deliberately: same keys,
         *  same meanings, same free-list ORDER (LIFO -- which is why
         *  @c visit_free_list exists rather than the printer deriving the free
         *  set from the cleared slots).  Only @c _name_ changed, from
         *  "RootSetInfo" to "RootSet", because the type it named is gone.
         *
         *  What DID change is WHEN the state is read: at print time, not at
         *  snapshot time.  @c AllocFlywheel::snapshot() no longer exists, so
         *  that is now the only reading there is.
         **/
        class JsonPrinter_RootSet : public JsonPrinter {
        public:
            using RootSet = xo::facet::DHandleArena<xo::facet::ObjectSlot>;

            JsonPrinter_RootSet(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                const RootSet * rs = this->check_recover_native<RootSet>(tp, p_os);

                if (!rs)
                    return;

                *p_os << "{"
                      << "\"_name_\": " << quot("RootSet")
                      << ", \"size\": " << rs->strong_size()
                      << ", \"capacity\": " << rs->strong_capacity()
                      << ", \"live\": " << rs->strong_root_count();

                *p_os << ", \"free\": [";
                {
                    bool first = true;

                    rs->visit_free_list([p_os, &first](std::size_t ix) {
                            if (!first)
                                *p_os << ", ";
                            first = false;
                            *p_os << ix;
                        });
                }
                *p_os << "]";

                /* every slot, cleared ones included, so a consumer reads a
                 * slot's index from its position here -- which is also what
                 * `free' indexes into.  Each element goes through
                 * JsonPrinter_ObjectSlot; a cleared slot renders as null.
                 */
                *p_os << ", \"slots\": [";
                {
                    bool first = true;
                    PrintJson const * pjson = this->pjson();

                    rs->visit_object_slots(
                        [pjson, p_os, &first](const xo::facet::ObjectSlot & slot) {
                            if (!first)
                                *p_os << ", ";
                            first = false;

                            pjson->print_aux(
                                TaggedPtr(Reflect::require<xo::facet::ObjectSlot>(),
                                          const_cast<xo::facet::ObjectSlot *>(&slot)),
                                p_os);
                        });
                }
                *p_os << "]}";
            } /*print_json*/
        }; /*JsonPrinter_RootSet*/


        /** @brief json printer for a whole AllocFlywheel -- one frame
         *
         *  This is the FRAME ENVELOPE, and it is a printer rather than a
         *  reflected struct because the last of those was retired on
         *  2026-09-22.
         *
         *  The lineage is worth knowing, because it went one layer at a time
         *  and each layer looked necessary while it stood:
         *
         *  | retired | was | replaced by |
         *  |---|---|---|
         *  | @c PoolInfo     | copy of MemorySizeInfo | reflecting MemorySizeInfo directly |
         *  | @c SlotInfo     | shadow of an erased fop | ObjectSlot + JsonPrinter_ObjectSlot |
         *  | @c RootSetInfo  | copy of the root set | visit_object_slots + JsonPrinter_RootSet |
         *  | @c FlywheelInfo | envelope struct | this |
         *
         *  What they had in common: each described a thing that could already
         *  describe itself, and each cost a second place to keep in step.  A
         *  printer is the right tool for a view model whose fields are
         *  COMPUTED -- @c capacity and @c live are not members of anything,
         *  and reflection can only name members.
         *
         *  A frame reads the flywheel at PRINT time.  There is no snapshot to
         *  go stale, which is the upside of the same property that made
         *  FlywheelInfo::strong_ a borrowed pointer before it went.
         **/
        class JsonPrinter_AllocFlywheel : public JsonPrinter {
        public:
            using AllocFlywheel = xo::facet::AllocFlywheel;

            JsonPrinter_AllocFlywheel(PrintJson const * pjson) : JsonPrinter(pjson) {}

            virtual void print_json(TaggedPtr tp,
                                    std::ostream * p_os) const override {
                using xo::mm::MemorySizeInfo;

                const AllocFlywheel * fw
                    = this->check_recover_native<AllocFlywheel>(tp, p_os);

                if (!fw)
                    return;

                PrintJson const * pjson = this->pjson();

                *p_os << "{" << "\"_name_\": " << quot("Flywheel");

                /* every pool the store owns, in the order it reports them:
                 * the storage arena first, then the root set and its free
                 * list.  pool_v_[0] is the arena a slot's offset is relative
                 * to -- though JsonPrinter_ObjectSlot recovers that per slot
                 * via DArena::obj2arena rather than relying on the order.
                 *
                 * MemorySizeInfo goes through REFLECTION, not by hand: it is
                 * already the right shape, so restating its fields here would
                 * be the very thing this printer exists to stop doing.
                 */
                *p_os << ", \"pools\": [";
                {
                    bool first = true;

                    fw->visit_pools([pjson, p_os, &first](const MemorySizeInfo & x) {
                            if (!first)
                                *p_os << ", ";
                            first = false;

                            pjson->print_aux(
                                TaggedPtr(Reflect::require<MemorySizeInfo>(),
                                          const_cast<MemorySizeInfo *>(&x)),
                                p_os);
                        });
                }
                *p_os << "]";

                /* the root set, via JsonPrinter_RootSet */
                *p_os << ", \"strong\": ";
                {
                    const auto & rs = fw->strong_root_set();

                    pjson->print_aux(
                        TaggedPtr(Reflect::require<AllocFlywheel::HandleStore>(),
                                  const_cast<AllocFlywheel::HandleStore *>(&rs)),
                        p_os);
                }

                *p_os << "}";
            } /*print_json*/
        }; /*JsonPrinter_AllocFlywheel*/

        namespace {
            /** describe MemorySizeInfo to xo-reflect, and install the three
             *  flywheel printers.
             *
             *  The reflection lives HERE, beside the printer that needs it,
             *  rather than in a free function a caller has to remember.  It
             *  was @c xo::facet::reflect_flywheel_info in xo-object2 until
             *  2026-09-22 -- homeless, because xo-facet owns AllocFlywheel but
             *  cannot reach a StructReflector, and its own doc said it was
             *  expected to move to wherever the consumer landed.  This is that
             *  consumer.
             *
             *  Member names are given EXPLICITLY rather than through
             *  REFLECT_MEMBER, which would derive the json key from the c++
             *  member name (`resource_name_' -> "resource_name").  The keys
             *  are a wire contract with a browser; house style for a member
             *  must not be able to rename them.
             *
             *  @c detail_ is deliberately absent: a pointer into the stack
             *  frame of whoever ran the visit, and almost always null.  If the
             *  per-type histogram is ever wanted it belongs in its own report
             *  at its own cadence.  (Raw pointers ARE reflected now -- see
             *  .xo-backlog/xo-reflect/issues/01 -- so this omission is a
             *  curation choice, not a limitation.)
             **/
            void
            provide_flywheel_printers(PrintJson * p_json)
            {
                using xo::mm::MemorySizeInfo;
                using xo::reflect::StructReflector;
                using RootSet = JsonPrinter_RootSet::RootSet;
                using AllocFlywheel = xo::facet::AllocFlywheel;

                {
                    /* idempotent: StructReflector's completion flag is
                     * per-type and static
                     */
                    StructReflector<MemorySizeInfo> sr;

                    sr.reflect_member("name", &MemorySizeInfo::resource_name_);
                    sr.reflect_member("used", &MemorySizeInfo::used_);
                    sr.reflect_member("allocated", &MemorySizeInfo::allocated_);
                    sr.reflect_member("committed", &MemorySizeInfo::committed_);
                    sr.reflect_member("reserved", &MemorySizeInfo::reserved_);
                    sr.reflect_member("lo", &MemorySizeInfo::lo_);
                    sr.reflect_member("hi", &MemorySizeInfo::hi_);

                    sr.require_complete();
                }

                {
                    std::unique_ptr<JsonPrinter> printer(new JsonPrinter_ObjectSlot(p_json));
                    p_json->provide_printer(Reflect::require<xo::facet::ObjectSlot>(),
                                            std::move(printer));
                }
                {
                    std::unique_ptr<JsonPrinter> printer(new JsonPrinter_RootSet(p_json));
                    /* the POINTEE; a const RootSet* reaches it through
                     * print_generic_pointer
                     */
                    p_json->provide_printer(Reflect::require<RootSet>(),
                                            std::move(printer));
                }
                {
                    std::unique_ptr<JsonPrinter> printer(new JsonPrinter_AllocFlywheel(p_json));
                    p_json->provide_printer(Reflect::require<AllocFlywheel>(),
                                            std::move(printer));
                }
            } /*provide_flywheel_printers*/
        } /*namespace*/

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

            /* MemoryNameStr (=flatstring<48>) and NameStr (=flatstring<32>) are
             * the sizes that appear in reflected structs today; re-derive
             * rather than trusting this list:
             *   grep -rn "using .*Str = flatstring" xo-facet xo-arena
             */
            provide_flatstring_printer<32>(this);
            provide_flatstring_printer<48>(this);

            provide_address_printer(this);

            provide_flywheel_printers(this);

            provide_utc_nanos_printer(this);
        } /*provide_std_printers*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonPrinter.cpp */
