/** @file objectmodel.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "FacetUtestAppcx.hpp"
#include "xo/facet/FacetRegistry.hpp"
#include "xo/facet/ObjectHandle.hpp"
#include "xo/facet/Top.hpp"
#include "xo/facet/OObject.hpp"
#include "xo/facet/RRouter.hpp"
#include "xo/facet/facet.hpp"
#include "xo/facet/facet_implementation.hpp"
#include "xo/facet/obj.hpp"
#include "xo/facet/typeseq.hpp"
#include <catch2/catch.hpp>
#include <vector>
#include <new>
#include <cassert>
#include <cmath>
#include <cstring>
#include <numbers>

namespace xo {
    using xo::FacetAppcx;
    using xo::carries_facet_appcx;
    using xo::carries_indentlog2_appcx;
    using xo::facet::ATop;
    using xo::facet::Opaque;
    using xo::facet::AllocFlywheel;
    using xo::facet::DObjectHandle;
    using xo::facet::valid_abstract_facet;
    using xo::facet::valid_facet_implementation;
    using xo::facet::FacetImplementation;
    using xo::facet::FacetRegistry;
    using xo::facet::DVariantPlaceholder;
    using xo::facet::OObject;
    using xo::facet::valid_object_router;
    using xo::facet::RoutingType;
    using xo::facet::typeseq;
    using xo::facet::obj;
    using xo::facet::with_facet;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;

    // ------ AComplex -----

    /** abstract interface for a complex number.
     *
     *  Inherits ATop, like every facet: that is what lets obj<AComplex,DRepr>
     *  narrow to the obj<ATop> a root slot holds.  ATop supplies _typeseq()
     *  and _drop(); the latter is what destruct_data() below predates.
     **/
    struct AComplex : public ATop {
        using TypeErasedIface = struct IComplex_Any;

        virtual double xcoord(void * data) const = 0;
        virtual double ycoord(void * data) const = 0;
        virtual double argument(void * data) const = 0;
        virtual double magnitude(void * data) const = 0;

        virtual void destruct_data(void * data) const = 0;

        static bool _valid;
    };

    bool
    AComplex::_valid = valid_abstract_facet<AComplex>();

    // ----- IComplex_Impl -----

    template <typename DRepr>
    struct IComplex_Impl;

    template <typename DRepr>
    struct IComplex_Xfer : public AComplex {
        // parallel interface to AComplex, but with specific data type
        using Impl = IComplex_Impl<DRepr>;

        // from AComplex

        virtual typeseq _typeseq() const noexcept final override { return s_typeseq; }
        virtual void _drop(Opaque data) const noexcept final override { Impl::destruct_data(*(DRepr*)data); }

        virtual double xcoord(void * data) const final override { return Impl::xcoord(*(DRepr*)data); }
        virtual double ycoord(void * data) const final override { return Impl::ycoord(*(DRepr*)data); }
        virtual double argument(void * data) const final override { return Impl::argument(*(DRepr*)data); }
        virtual double magnitude(void * data) const final override { return Impl::magnitude(*(DRepr*)data); }

        virtual void destruct_data(void * data) const final override { Impl::destruct_data(*(DRepr*)data); }

        static typeseq s_typeseq;
        static bool _valid;
    };

    template <typename DRepr>
    typeseq
    IComplex_Xfer<DRepr>::s_typeseq = typeseq::id<DRepr>();

    template <typename DRepr>
    bool
    IComplex_Xfer<DRepr>::_valid = valid_facet_implementation<AComplex, IComplex_Xfer>;

    namespace facet {
        template <typename DRepr>
        struct FacetImplementation<AComplex, DRepr> {
           using ImplType = IComplex_Xfer<DRepr>;
        };
    }

    // ----- IComplex_Any -----

    /** type-erased implementation of AComplex, for runtime polymorphism
     *  Usable by (and only by) overwriting with a typed implementation,
     *  such as IComplex_RectCoords or IComplex_PolarCoords.
     **/
    struct IComplex_Any : public AComplex {
        virtual typeseq _typeseq() const noexcept final override { return s_typeseq; }
        virtual void _drop(Opaque) const noexcept final override { assert(false); }

        virtual double xcoord(void *) const final override { assert(false); return 0.0; }
        virtual double ycoord(void *) const final override { assert(false); return 0.0; }
        virtual double argument(void *) const final override { assert(false); return 0.0; }
        virtual double magnitude(void *) const final override { assert(false); return 0.0; }

        virtual void destruct_data(void *) const final override { assert(false); }

        static typeseq s_typeseq;
        static bool _valid;
    };

    typeseq
    IComplex_Any::s_typeseq = typeseq::id<DVariantPlaceholder>();

    bool
    IComplex_Any::_valid = valid_facet_implementation<AComplex, IComplex_Any>();

    namespace facet {
        template <>
        struct FacetImplementation<AComplex, DVariantPlaceholder> {
            using ImplType = IComplex_Any;
        };
    }

    // ----------------------------------------------------------------
    // AComplex, DPolarCoords
    //
    // complex number represented using polar coordinates (arg, mag)
    // ----------------------------------------------------------------

    struct DPolarCoords {
        DPolarCoords(double arg, double mag) : arg_{arg}, mag_{mag} {}

        double arg_;
        double mag_;
    };

    using IComplex_DPolarCoords = IComplex_Xfer<DPolarCoords>;

    template <>
    struct IComplex_Impl<DPolarCoords> {
        static double xcoord(DPolarCoords & self) { return self.mag_ * std::cos(self.arg_); }
        static double ycoord(DPolarCoords & self) { return self.mag_ * std::sin(self.arg_); }
        static double argument(DPolarCoords & self) { return self.arg_; }
        static double magnitude(DPolarCoords & self) { return self.mag_; }

        static void destruct_data(DPolarCoords & self) { self.~DPolarCoords(); }
    };

    // ----------------------------------------------------------------
    // AComplex, DRectCoords
    //
    // complex number represented using rectangular coordinates (x, y)
    // ----------------------------------------------------------------

    struct DRectCoords {
        DRectCoords(double x, double y) : x_{x}, y_{y} {}

        double x_;
        double y_;
    };

    using IComplex_DRectCoords = IComplex_Xfer<DRectCoords>;

    template <>
    struct IComplex_Impl<DRectCoords> {
        static double xcoord(DRectCoords & self) { return self.x_; }
        static double ycoord(DRectCoords & self) { return self.y_; }
        static double argument(DRectCoords & self) { return std::atan(self.y_ / self.x_); }
        static double magnitude(DRectCoords & self) {
            double x = self.x_;
            double y = self.y_;

            return std::sqrt(x*x + y*y);
        }

        static void destruct_data(DRectCoords & self) { self.~DRectCoords(); }
    };

    // ----------------------------------------------------------------
    // RComplex
    //
    // convenience router: supplies data argument to AComplex methods
    // ----------------------------------------------------------------

    template <typename Object>
    struct RComplex : public Object {
        using ObjectType = Object;

        RComplex() {}
        RComplex(Object::DataPtr data) : Object{std::move(data)} {}

        typeseq _typeseq() const { return Object::iface()->_typeseq(); }
        double xcoord() const { return Object::iface()->xcoord(Object::data()); }
        double ycoord() const { return Object::iface()->ycoord(Object::data()); }
        double argument() const { return Object::iface()->argument(Object::data()); }
        double magnitude() const { return Object::iface()->magnitude(Object::data()); }

        /** note: would prefer this to be constexpr, but seems infeasible asof gcc 14.3 **/
        static bool _valid;
    };

    template <typename Object>
    bool
    RComplex<Object>::_valid = valid_object_router<Object>();

    namespace facet {
        template <typename Object>
        struct RoutingFor<AComplex, Object> {
            using RoutingType = RComplex<Object>;
        };
    } /*namespace facet*/

    namespace ut {
        // ----- TESTS -----

        TEST_CASE("facet-1", "[facet]")
        {
            // AComplex passes abstract facet checks
            REQUIRE(AComplex::_valid);

            // IComplex_Any passes facet implementation checks
            REQUIRE(IComplex_Any::_valid);
        }

        TEST_CASE("xfer-polar-1", "[facet]")
        {
            IComplex_Impl<DPolarCoords> impl;
            DPolarCoords z1{0.0, 1.0};

            REQUIRE(decltype(impl)::xcoord(z1) == 1.0);
            REQUIRE(decltype(impl)::ycoord(z1) == 0.0);
            REQUIRE(decltype(impl)::argument(z1) == 0.0);
            REQUIRE(decltype(impl)::magnitude(z1) == 1.0);
        }

        TEST_CASE("xfer-rect-1", "[facet]")
        {
            IComplex_Impl<DRectCoords> impl;
            DRectCoords z1{1.0, 0.0};

            REQUIRE(decltype(impl)::xcoord(z1) == 1.0);
            REQUIRE(decltype(impl)::ycoord(z1) == 0.0);
            REQUIRE(decltype(impl)::argument(z1) == 0.0);
            REQUIRE(decltype(impl)::magnitude(z1) == 1.0);
        }

        TEST_CASE("oobject-polar-1", "[facet]")
        {
            using Object = OObject<AComplex, DPolarCoords>;

            DPolarCoords z1{0.0, 1.0};
            Object obj(&z1);

            REQUIRE(obj.iface()->xcoord(obj.data()) == 1.0);
            REQUIRE(obj.iface()->ycoord(obj.data()) == 0.0);
            REQUIRE(obj.iface()->argument(obj.data()) == 0.0);
            REQUIRE(obj.iface()->magnitude(obj.data()) == 1.0);
        }

        TEST_CASE("oobject-rect-1", "[facet]")
        {
            using Object = OObject<AComplex, DRectCoords>;
            DRectCoords z1{1.0, 0.0};

            Object obj(&z1);

            REQUIRE(obj.iface()->xcoord(obj.data()) == 1.0);
            REQUIRE(obj.iface()->ycoord(obj.data()) == 0.0);
            REQUIRE(obj.iface()->argument(obj.data()) == 0.0);
            REQUIRE(obj.iface()->magnitude(obj.data()) == 1.0);
        }

        TEST_CASE("rrouter-polar-1", "[facet]")
        {
            using Router = RoutingType<AComplex, OObject<AComplex, DPolarCoords>>;

            DPolarCoords z1{0.0, 1.0};
            Router obj(&z1);

            REQUIRE(obj.xcoord() == 1.0);
            REQUIRE(obj.ycoord() == 0.0);
            REQUIRE(obj.argument() == 0.0);
            REQUIRE(obj.magnitude() == 1.0);

        }

        TEST_CASE("rrouter-rect-1", "[facet]")
        {
            using Router = RoutingType<AComplex, OObject<AComplex, DRectCoords>>;

            DRectCoords z1{1.0, 0.0};
            Router obj(&z1);

            REQUIRE(obj.xcoord() == 1.0);
            REQUIRE(obj.ycoord() == 0.0);
            REQUIRE(obj.argument() == 0.0);
            REQUIRE(obj.magnitude() == 1.0);

        }

        TEST_CASE("rrouter-any-1", "[facet]")
        {
            using Router = RoutingType<AComplex, OObject<AComplex>>;

            // variant!
            Router var1;

            REQUIRE(var1.iface() != nullptr);
            REQUIRE(var1.data() == nullptr);

            {
                DRectCoords z1{1.0, 0.0};

                var1.from_data(&z1);

                REQUIRE(var1.iface() != nullptr);
                REQUIRE((void*)var1.data() == (void*)&z1);

                REQUIRE(var1.xcoord() == z1.x_);
                REQUIRE(var1.ycoord() == z1.y_);

                REQUIRE(var1.xcoord() == 1.0);
                REQUIRE(var1.ycoord() == 0.0);
                REQUIRE(var1.argument() == 0.0);
                REQUIRE(var1.magnitude() == 1.0);

                REQUIRE(var1.downcast<DPolarCoords>() == nullptr);
                REQUIRE(var1.downcast<DRectCoords>() == &z1);
            }

            {
                DPolarCoords z2{0.0, 1.0};

                var1.from_data(&z2);

                REQUIRE(var1.iface() != nullptr);
                REQUIRE((void*)var1.data() == (void*)&z2);

                REQUIRE(var1.argument() == z2.arg_);
                REQUIRE(var1.magnitude() == z2.mag_);

                REQUIRE(var1.xcoord() == 1.0);
                REQUIRE(var1.ycoord() == 0.0);
                REQUIRE(var1.argument() == 0.0);
                REQUIRE(var1.magnitude() == 1.0);

                REQUIRE(var1.downcast<DRectCoords>() == nullptr);
                REQUIRE(var1.downcast<DPolarCoords>() == &z2);
            }
        }

        TEST_CASE("obj-rect-1", "[facet]")
        {
            DRectCoords z1{1.0, 0.0};
            auto z1o = with_facet<AComplex>::mkobj(&z1);

            static_assert(std::is_same_v<decltype(z1o)::FacetType, AComplex>);
            static_assert(std::is_same_v<decltype(z1o)::DataType, DRectCoords>);

            REQUIRE(z1o._typeseq() == typeseq::id<DRectCoords>());
            REQUIRE(z1o.xcoord() == 1.0);
            REQUIRE(z1o.ycoord() == 0.0);
            REQUIRE(z1o.argument() == 0.0);
            REQUIRE(z1o.magnitude() == 1.0);

            // downcast isn't part of interface for non-variant DRepr
            // REQUIRE(z1o.downcast<DRectCoords>() == &z1);

            double h = 0.5 * std::sqrt(2.0);
            DRectCoords z2{h, h};

            z1o.from_data(&z2);

            REQUIRE(z1o.data() != &z1);
            REQUIRE(z1o.data() == &z2);

            REQUIRE(z1o.xcoord() == h);
            REQUIRE(z1o.ycoord() == h);
            REQUIRE(z1o.argument() == 0.25 * std::numbers::pi);
            REQUIRE(z1o.magnitude() == 1.0);

            *z1o.data() = z1;

            REQUIRE(z1o.data() == &z2);
            REQUIRE(z1o.xcoord() == 1.0);
            REQUIRE(z1o.ycoord() == 0.0);
            REQUIRE(z1o.argument() == 0.0);
            REQUIRE(z1o.magnitude() == 1.0);
        }

        TEST_CASE("obj-any-1", "[facet]")
        {
            obj<AComplex> var1;

            REQUIRE(!var1);
            REQUIRE(var1.iface() != nullptr);
            REQUIRE(var1.data() == nullptr);

            DRectCoords z1{1.0, 0.0};
            obj<AComplex, DRectCoords> z1o(&z1);

            REQUIRE(z1o);
        }

        TEST_CASE("registry-1", "[facet][registry]")
        {
            auto & registry = FacetRegistry::instance();

            // register implementations
            FacetRegistry::register_impl<AComplex, DRectCoords>();
            FacetRegistry::register_impl<AComplex, DPolarCoords>();

            REQUIRE(registry.contains(typeseq::id<AComplex>(),
                                       typeseq::id<DRectCoords>()));
            REQUIRE(registry.contains(typeseq::id<AComplex>(),
                                       typeseq::id<DPolarCoords>()));
        }

        TEST_CASE("registry-lookup-1", "[facet][registry]")
        {
            // ensure registered
            FacetRegistry::register_impl<AComplex, DRectCoords>();
            FacetRegistry::register_impl<AComplex, DPolarCoords>();

            // runtime lookup using typeseq
            typeseq  rect_id = typeseq::id<DRectCoords>();
            typeseq polar_id = typeseq::id<DPolarCoords>();

            const AComplex *  rect_impl = FacetRegistry::impl_for<AComplex>(rect_id);
            const AComplex * polar_impl = FacetRegistry::impl_for<AComplex>(polar_id);

            REQUIRE( rect_impl != nullptr);
            REQUIRE(polar_impl != nullptr);

            // use implementations
            DRectCoords  z1{1.0, 0.0};
            DPolarCoords z2{0.0, 1.0};

            REQUIRE(rect_impl->xcoord(&z1) == 1.0);
            REQUIRE(rect_impl->ycoord(&z1) == 0.0);

            REQUIRE(polar_impl->xcoord(&z2) == 1.0);
            REQUIRE(polar_impl->ycoord(&z2) == 0.0);
        }

        TEST_CASE("registry-cross-facet", "[facet][registry]")
        {
            // simulate the DList::pretty() use case:
            // given obj<AComplex> with unknown repr type,
            // look up implementation at runtime

            FacetRegistry::register_impl<AComplex, DRectCoords>();
            FacetRegistry::register_impl<AComplex, DPolarCoords>();

            // create type-erased objects
            DRectCoords z1{1.0, 0.0};
            DPolarCoords z2{0.0, 2.0};

            obj<AComplex, DRectCoords> obj1(&z1);
            obj<AComplex, DPolarCoords> obj2(&z2);

            // simulate: only know typeseq at runtime
            typeseq repr1 = obj1._typeseq();
            typeseq repr2 = obj2._typeseq();

            // lookup implementations
            const AComplex * impl1 = FacetRegistry::impl_for<AComplex>(repr1);
            const AComplex * impl2 = FacetRegistry::impl_for<AComplex>(repr2);

            REQUIRE(impl1 != nullptr);
            REQUIRE(impl2 != nullptr);

            // use via runtime-looked-up implementation
            REQUIRE(impl1->magnitude(obj1.data()) == 1.0);
            REQUIRE(impl2->magnitude(obj2.data()) == 2.0);
        }

        TEST_CASE("registry-not-found", "[facet][registry]")
        {
            // lookup for unregistered type returns nullptr
            struct DUnknown {};

            const AComplex * impl = FacetRegistry::impl_for<AComplex>(typeseq::id<DUnknown>());

            REQUIRE(impl == nullptr);
        }

        /* is_fomo distinguishes a faceted object from an ordinary c++ type,
         * so generic code can select facet-aware behavior at compile time
         * (e.g. pivoting via FacetRegistry::try_variant, which is only
         * meaningful for a faceted object).
         */
        TEST_CASE("is_fomo", "[objectmodel][is_fomo]") {
            /* faceted: both the typed and the type-erased (variant) form */
            static_assert(xo::facet::is_fomo_v<obj<AComplex, DRectCoords>>);
            static_assert(xo::facet::is_fomo_v<obj<AComplex, DPolarCoords>>);
            static_assert(xo::facet::is_fomo_v<obj<AComplex>>);
            static_assert(xo::facet::is_fomo_v<xo::facet::vt<AComplex>>);

            /* not faceted: the bare representation, and ordinary types */
            static_assert(!xo::facet::is_fomo_v<DRectCoords>);
            static_assert(!xo::facet::is_fomo_v<DRectCoords *>);
            static_assert(!xo::facet::is_fomo_v<AComplex>);
            static_assert(!xo::facet::is_fomo_v<int>);
            static_assert(!xo::facet::is_fomo_v<void (*)(int)>);

            /* exported into namespace xo as well */
            static_assert(xo::is_fomo_v<obj<AComplex, DRectCoords>>);
            static_assert(!xo::is_fomo_v<int>);

            /* the trait is a real type, not just the _v alias */
            static_assert(xo::facet::is_fomo<obj<AComplex>>::value);

            SUCCEED("is_fomo static assertions hold");
        }

        /** The witness chain, checked at compile time.
         *
         *  Each of these types can hand over the context it was built from, so
         *  holding one is evidence the corresponding subsystem was configured.
         *  Asserted here rather than trusted, because the two python __repr__
         *  bindings render unconditionally on exactly this reasoning.
         *
         *  The negative cases matter as much as the positive ones: a concept
         *  that accidentally accepted everything, or that nothing satisfied,
         *  would pass silently.
         **/
        TEST_CASE("witness-chain", "[facet][witness]")
        {
            /* the contexts themselves */
            static_assert(carries_indentlog2_appcx<xo::FacetAppcx>);

            /* a flywheel retains the context it was made with, and forwards */
            static_assert(carries_facet_appcx<AllocFlywheel>);
            static_assert(carries_indentlog2_appcx<AllocFlywheel>);

            /* and a handle reaches both through its flywheel */
            static_assert(carries_facet_appcx<DObjectHandle<AComplex, DRectCoords>>);
            static_assert(carries_indentlog2_appcx<DObjectHandle<AComplex, DRectCoords>>);

            /* not satisfied by types that cannot attest */
            static_assert(!carries_facet_appcx<int>);
            static_assert(!carries_indentlog2_appcx<int>);
            static_assert(!carries_facet_appcx<DRectCoords>);

            SUCCEED("witness chain holds");
        }

        /** DObjectHandle over a facet OTHER than ATop.
         *
         *  The flywheel's root slot holds obj<ATop>, so a handle has to narrow
         *  on the way in and recover the typed facet on the way out.  Nothing
         *  instantiated DObjectHandle before this, so neither direction had
         *  ever been compiled.
         **/
        TEST_CASE("objecthandle-nontop-facet", "[facet][objecthandle]")
        {
            FacetAppcx & facet_appcx = FacetUtestAppcx::appcx().cx<S_facet_tag>();
            ArenaConfig storage_cfg{ .name_ = flatstring("utest.oh.storage"), .size_ = 16*1024 };
            ArenaConfig strong_cfg { .name_ = flatstring("utest.oh.strong"),  .size_ =  4*1024 };

            rp<AllocFlywheel> fw = AllocFlywheel::make_app(facet_appcx, storage_cfg, strong_cfg);

            REQUIRE(fw.get() != nullptr);

            /* allocate the representation from the flywheel's own arena,
             * as make_strong_ref requires
             */
            auto * mem = fw->storage().alloc(typeseq::id<DRectCoords>(), sizeof(DRectCoords));

            REQUIRE(mem != nullptr);

            DRectCoords * p = new (mem) DRectCoords(3.0, 4.0);

            auto h = DObjectHandle<AComplex, DRectCoords>::make_strong_ref
                (fw, obj<AComplex, DRectCoords>(p));

            /* recovered obj must address the same representation ... */
            REQUIRE(h._native().data() == p);

            /* ... and route AComplex methods to the DRectCoords implementation */
            REQUIRE(h._native().xcoord() == 3.0);
            REQUIRE(h._native().ycoord() == 4.0);

            /* the narrowed slot kept the concrete runtime type */
            REQUIRE(h._native()._typeseq() == typeseq::id<DRectCoords>());
        }

        namespace {
            /** flywheel with a DELIBERATELY small strong set, so a loop can
             *  outrun it in a test-sized number of iterations.
             *
             *  4*1024 would hold 256 obj<ATop>, which is too many to exhaust
             *  legibly; this holds far fewer, and the tests below derive the
             *  actual number rather than assuming one.
             **/
            rp<AllocFlywheel> make_small_flywheel(const char * tag) {
                FacetAppcx & facet_appcx = FacetUtestAppcx::appcx().cx<S_facet_tag>();

                ArenaConfig storage_cfg{ .name_ = ArenaNameStr::sprintf("%s.storage", tag),
                                         .size_ = 64*1024 };
                ArenaConfig strong_cfg { .name_ = ArenaNameStr::sprintf("%s.strong", tag),
                                         .size_ = 64 };

                return AllocFlywheel::make_app(facet_appcx, storage_cfg, strong_cfg);
            }

            /** a DRectCoords in @p fw's arena, as make_strong_ref requires **/
            DRectCoords * alloc_rect(rp<AllocFlywheel> & fw, double x, double y) {
                auto * mem = fw->storage().alloc(typeseq::id<DRectCoords>(),
                                                 sizeof(DRectCoords));
                REQUIRE(mem != nullptr);

                return new (mem) DRectCoords(x, y);
            }

            /** the root set's slots, in index order -- cleared ones included.
             *
             *  This is what a reader gets since RootSetInfo was retired
             *  (2026-09-21): a walk, not a copy.  Collecting into a vector
             *  here is the TEST's choice, so the assertions below can be
             *  written by index; JsonPrinter_RootSet streams instead.
             **/
            std::vector<xo::facet::ObjectSlot> slots_of(const AllocFlywheel & fw) {
                std::vector<xo::facet::ObjectSlot> retval;

                fw.visit_object_slots([&retval](const xo::facet::ObjectSlot & slot) {
                        retval.push_back(slot);
                    });

                return retval;
            }

            /** the free list, oldest release first -- reuse takes the LAST **/
            std::vector<std::size_t> free_list_of(const AllocFlywheel & fw) {
                std::vector<std::size_t> retval;

                fw.visit_free_list([&retval](std::size_t ix) {
                        retval.push_back(ix);
                    });

                return retval;
            }
        }

        TEST_CASE("objecthandle-releases-its-root", "[facet][objecthandle][freelist]")
        {
            using xo::facet::DObjectHandle;
            using H = DObjectHandle<AComplex, DRectCoords>;

            rp<AllocFlywheel> fw = make_small_flywheel("utest.rel");

            REQUIRE(fw->strong_root_count() == 0);

            {
                auto h = H::make_strong_ref(fw, obj<AComplex, DRectCoords>
                                                (alloc_rect(fw, 3.0, 4.0)));

                REQUIRE(fw->strong_root_count() == 1);

                /* silence the unused-variable warning without reading through
                 * the handle after the scope closes
                 */
                REQUIRE(h._native().xcoord() == 3.0);
            }

            /* ~ObjectHandleBase returned the slot.  Before 2026-09-13 the dtor
             * was `= default' and this stayed 1 for the life of the flywheel,
             * which is what made python refcounting unobservable.
             */
            REQUIRE(fw->strong_root_count() == 0);
        }

        TEST_CASE("objecthandle-move-does-not-double-release",
                  "[facet][objecthandle][freelist]")
        {
            using xo::facet::DObjectHandle;
            using H = DObjectHandle<AComplex, DRectCoords>;

            rp<AllocFlywheel> fw = make_small_flywheel("utest.mv");

            {
                auto h = H::make_strong_ref(fw, obj<AComplex, DRectCoords>
                                                (alloc_rect(fw, 1.0, 2.0)));
                REQUIRE(fw->strong_root_count() == 1);

                /* one slot, one owner: the moved-from handle must not release
                 * when it dies at the end of this scope
                 */
                auto h2 = std::move(h);

                REQUIRE(fw->strong_root_count() == 1);
                REQUIRE(h2._native().xcoord() == 1.0);
            }

            REQUIRE(fw->strong_root_count() == 0);
        }

        TEST_CASE("handle-loop-reuses-slots", "[facet][objecthandle][freelist]")
        {
            using xo::facet::DObjectHandle;
            using H = DObjectHandle<AComplex, DRectCoords>;

            rp<AllocFlywheel> fw = make_small_flywheel("utest.loop");

            /* how many slots this flywheel actually has.  Derived, not assumed:
             * an arena rounds its reservation up to a page, so the strong set
             * holds rather more than the 64 bytes asked for.
             */
            std::size_t capacity = 0;
            {
                std::vector<H> hold;

                /* fill it: stop when add_strong_ref stops handing out slots.
                 * NB that is a silent truncation rather than an error -- see
                 * .xo-backlog/pyobject2/issues/09.
                 */
                DRectCoords * p = alloc_rect(fw, 0.0, 0.0);

                while (true) {
                    auto h = H::make_strong_ref(fw, obj<AComplex, DRectCoords>(p));

                    if (h._impl_handle() == nullptr)
                        break;

                    hold.push_back(std::move(h));
                }

                capacity = hold.size();

                REQUIRE(capacity > 0);
                REQUIRE(fw->strong_root_count() == capacity);
            }

            REQUIRE(fw->strong_root_count() == 0);

            /* the point of the free list: far more handles than the set can
             * hold, one at a time, without exhausting it.
             *
             * ONE representation, rooted repeatedly.  Allocating a fresh
             * DRectCoords per iteration exhausts the STORAGE arena instead --
             * which this test would then report as a free-list failure.  The
             * two resources are independent and only the root set is under
             * test here.
             */
            DRectCoords * p = alloc_rect(fw, 1.0, 2.0);

            for (std::size_t i = 0; i < 20 * capacity; ++i) {
                auto h = H::make_strong_ref(fw, obj<AComplex, DRectCoords>(p));

                REQUIRE(h._impl_handle() != nullptr);
                REQUIRE(fw->strong_root_count() == 1);
            }

            REQUIRE(fw->strong_root_count() == 0);
        }

        TEST_CASE("flywheel-snapshot-reports-live-slots",
                  "[facet][objecthandle][snapshot]")
        {
            using H = DObjectHandle<AComplex, DRectCoords>;

            rp<AllocFlywheel> fw = make_small_flywheel("utest.snap");

            /* registered so the snapshot can NAME the representation; without
             * it typeseq_ is still reported and type_ is the sentinel
             */
            xo::facet::FacetRegistry::register_impl<AComplex, DRectCoords>();

            {
                /* the pools the store reports, in its order.
                 *
                 * Read through visit_pools rather than a snapshot: FlywheelInfo
                 * was retired 2026-09-21 along with AllocFlywheel::snapshot(),
                 * and a frame is now produced by JsonPrinter_AllocFlywheel
                 * walking these same visitors.  So this asserts on the path the
                 * wire actually takes.
                 */
                std::vector<std::string> pool_name_v;

                fw->visit_pools([&pool_name_v](const xo::mm::MemorySizeInfo & x) {
                        pool_name_v.push_back(std::string(x.resource_name_));
                    });

                REQUIRE(pool_name_v.size() == 3);
                REQUIRE(pool_name_v[0] == std::string("utest.snap.storage"));
                REQUIRE(pool_name_v[1] == std::string("utest.snap.strong"));
                REQUIRE(pool_name_v[2] == std::string("utest.snap.strong-free"));

                REQUIRE(fw->strong_root_count() == 0);
                /* the walk covers the high-water mark, not the population:
                 * cleared slots are visited too, so a slot's index is its
                 * position in the walk
                 */
                REQUIRE(slots_of(*fw.get()).size() == fw->strong_size());
                REQUIRE(fw->strong_capacity() > 0);
            }

            DRectCoords * p0 = alloc_rect(fw, 3.0, 4.0);
            DRectCoords * p1 = alloc_rect(fw, 5.0, 6.0);

            auto h0 = H::make_strong_ref(fw, obj<AComplex, DRectCoords>(p0));
            auto h1 = H::make_strong_ref(fw, obj<AComplex, DRectCoords>(p1));

            {
                auto slot_v = slots_of(*fw.get());

                REQUIRE(fw->strong_root_count() == 2);
                REQUIRE(slot_v.size() == 2);
                REQUIRE(free_list_of(*fw.get()).empty());

                /* the slots ARE the handles' targets -- no shadow struct.
                 * SlotInfo held a derived (ix, typeseq, type, offset) until
                 * 2026-09-20; an ObjectSlot answers all four itself, and
                 * JsonPrinter_ObjectSlot resolves the offset from the pointer
                 * via DArena::obj2arena rather than being handed a base.
                 */
                REQUIRE(slot_v[h0.object_ix()].opaque_data() == p0);
                REQUIRE(slot_v[h1.object_ix()].opaque_data() == p1);

                REQUIRE(slot_v[h0.object_ix()]._typeseq()
                        == typeseq::id<DRectCoords>());

                /* position in the walk IS the index, which is what lets the
                 * free list index into it
                 */
                REQUIRE(h0.object_ix() != h1.object_ix());
            }

            /* a released slot leaves the report, and shows up on the free list
             * instead -- the frame tracks occupancy, not the high-water mark
             */
            auto ix0 = h0.object_ix();
            fw->remove_strong_ref(ix0);

            {
                auto slot_v = slots_of(*fw.get());

                REQUIRE(fw->strong_root_count() == 1);
                /* the walk still covers every slot; the released one is empty
                 * rather than absent, and renders as json null
                 */
                REQUIRE(slot_v.size() == 2);
                REQUIRE(!slot_v[ix0]);
                REQUIRE(slot_v[h1.object_ix()].opaque_data() == p1);

                auto free_v = free_list_of(*fw.get());

                REQUIRE(free_v.size() == 1);
                REQUIRE(free_v[0] == ix0);

                /* strong_size() is the high-water mark and does NOT fall */
                REQUIRE(fw->strong_size() == 2);
            }
        }

        TEST_CASE("double-release-does-not-share-a-slot",
                  "[facet][objecthandle][freelist]")
        {
            using xo::facet::DObjectHandle;
            using H = DObjectHandle<AComplex, DRectCoords>;

            rp<AllocFlywheel> fw = make_small_flywheel("utest.dbl");

            auto h = H::make_strong_ref(fw, obj<AComplex, DRectCoords>
                                            (alloc_rect(fw, 5.0, 6.0)));
            auto ix = h.object_ix();

            fw->remove_strong_ref(ix);
            REQUIRE(fw->strong_root_count() == 0);

            /* second release of the same index is a no-op.  Without the
             * emptiness guard it would push ix onto the free list twice, and
             * the two make_strong_ref calls below would collide on one slot.
             */
            fw->remove_strong_ref(ix);
            REQUIRE(fw->strong_root_count() == 0);

            auto a = H::make_strong_ref(fw, obj<AComplex, DRectCoords>
                                            (alloc_rect(fw, 7.0, 0.0)));
            auto b = H::make_strong_ref(fw, obj<AComplex, DRectCoords>
                                            (alloc_rect(fw, 8.0, 0.0)));

            REQUIRE(a.object_ix() != b.object_ix());
            REQUIRE(a._native().xcoord() == 7.0);
            REQUIRE(b._native().xcoord() == 8.0);

            /* h's dtor will release ix a third time; harmless for the same
             * reason, but the slot is now occupied by someone else, so this
             * asserts the guard holds where it matters
             */
        }
    }
}

/* end objectmodel.test.cpp */
