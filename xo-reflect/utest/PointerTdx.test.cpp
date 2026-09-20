/* file PointerTdx.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 *
 * reflection for raw pointers -- see .xo-backlog/xo-reflect/issues/01.
 *
 * Before that, a raw T* fell to EstablishTdx's primary template and reflected
 * as an opaque atom.  It now reflects the way xo::ref::rp<Object> does: 0
 * children when null, 1 otherwise.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::Metatype;

    namespace ut {
        namespace {
            struct DPtrProbe {
                double x_;
            };

            int probe_fn(double) { return 0; }
        }

        TEST_CASE("raw-pointer-reflects-as-a-pointer", "[reflect][rawpointer]") {
            DPtrProbe probe{1.5};
            DPtrProbe * p = &probe;

            TaggedPtr tp = Reflect::make_tp(&p);

            REQUIRE(tp.td()->metatype() == Metatype::mt_pointer);
            REQUIRE(tp.is_vector() == false);
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.n_child() == 1);

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.address() == &probe);
            REQUIRE(tp0.recover_native<DPtrProbe>() == &probe);
            REQUIRE(tp0.recover_native<DPtrProbe>()->x_ == 1.5);
        } /*TEST_CASE(raw-pointer-reflects-as-a-pointer)*/

        TEST_CASE("null-raw-pointer-has-no-children", "[reflect][rawpointer]") {
            DPtrProbe * p = nullptr;

            TaggedPtr tp = Reflect::make_tp(&p);

            REQUIRE(tp.td()->metatype() == Metatype::mt_pointer);
            /* the property that makes a pointer a 0-or-1 container rather than
             * an atom -- and what lets print_generic_pointer emit json null
             * without dereferencing
             */
            REQUIRE(tp.n_child() == 0);
        } /*TEST_CASE(null-raw-pointer-has-no-children)*/

        TEST_CASE("pointer-to-const-shares-one-pointee", "[reflect][rawpointer]") {
            /* typeid(const Foo*) and typeid(Foo*) DIFFER -- there the const is
             * not top-level -- so the two pointer types keep separate
             * descriptors.  Their POINTEE must still be one TypeDescr, or a
             * printer registered on Foo would not fire for a const Foo*.
             *
             * RawPointerTdx::pointee_t strips cv for exactly this.  Without
             * it, the pointee's canonical_name would also depend on which of
             * the two was established first, and a struct's json "_name_"
             * comes from that name.
             */
            auto td_mut = Reflect::require<DPtrProbe *>();
            auto td_con = Reflect::require<const DPtrProbe *>();

            REQUIRE(td_mut != td_con);

            REQUIRE(td_mut->metatype() == Metatype::mt_pointer);
            REQUIRE(td_con->metatype() == Metatype::mt_pointer);

            REQUIRE(td_mut->fixed_child_td(0) == Reflect::require<DPtrProbe>());
            REQUIRE(td_con->fixed_child_td(0) == Reflect::require<DPtrProbe>());

            /* and the pointee is reachable through the const pointer */
            DPtrProbe probe{2.5};
            const DPtrProbe * cp = &probe;

            TaggedPtr tp = Reflect::make_tp(&cp);

            REQUIRE(tp.n_child() == 1);
            REQUIRE(tp.get_child(0).address() == &probe);
        } /*TEST_CASE(pointer-to-const-shares-one-pointee)*/

        TEST_CASE("char-pointers-are-exempt", "[reflect][rawpointer]") {
            /* char strings are TEXT.  Full specialisations out-rank the T*
             * partial one, so these stay atoms and keep rendering as quoted
             * strings via provide_string_printer.
             *
             * If either reads mt_pointer, a string has become a
             * pointer-to-char and every consumer of it sees an array.
             */
            REQUIRE(Reflect::require<char *>()->metatype() == Metatype::mt_atomic);
            REQUIRE(Reflect::require<const char *>()->metatype() == Metatype::mt_atomic);
        } /*TEST_CASE(char-pointers-are-exempt)*/

        TEST_CASE("void-pointer-has-no-children", "[reflect][rawpointer]") {
            /* a void pointee has no representation to traverse into, so it
             * reports 0 children even when NON-null.  MemorySizeInfo::lo_/hi_
             * are const void* and reflected; they keep rendering as decimal
             * addresses because print_aux consults printer_map_ before the
             * metatype switch, so JsonPrinter_address still wins.
             */
            int x = 7;
            const void * p = &x;

            TaggedPtr tp = Reflect::make_tp(&p);

            REQUIRE(tp.td()->metatype() == Metatype::mt_pointer);
            REQUIRE(p != nullptr);
            REQUIRE(tp.n_child() == 0);
        } /*TEST_CASE(void-pointer-has-no-children)*/

        TEST_CASE("function-pointer-keeps-its-own-tdx", "[reflect][rawpointer]") {
            /* EstablishTdx<Retval (*)(Args...)> is a more specialised match
             * than EstablishTdx<T*>, and partial ordering prefers it.  This is
             * the safety claim the blanket specialisation rests on.
             */
            auto td = Reflect::require<decltype(&probe_fn)>();

            REQUIRE(td->metatype() == Metatype::mt_function);
            REQUIRE(td->metatype() != Metatype::mt_pointer);
        } /*TEST_CASE(function-pointer-keeps-its-own-tdx)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end PointerTdx.test.cpp */
