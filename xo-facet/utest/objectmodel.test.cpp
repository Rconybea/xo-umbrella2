/** @file objectmodel.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/facet/facet.hpp"
#include "xo/facet/facet_implementation.hpp"
#include "xo/facet/OObject.hpp"
#include "xo/facet/RRouter.hpp"
#include "xo/facet/typeseq.hpp"
#include "xo/facet/obj.hpp"
#include <catch2/catch.hpp>
#include <cmath>
#include <numbers>
#include <cassert>
#include <cstring>

namespace xo {
    using xo::facet::valid_abstract_facet;
    using xo::facet::valid_facet_implementation;
    using xo::facet::FacetImplementation;
    using xo::facet::DVariantPlaceholder;
    using xo::facet::OObject;
    using xo::facet::valid_object_router;
    using xo::facet::RoutingType;
    using xo::facet::typeseq;
    using xo::facet::obj;
    using xo::facet::with_facet;

    // ------ AComplex -----

    /** abstract interface for a complex number **/
    struct AComplex {
        using TypeErasedIface = struct IComplex_Any;

        /** RTTI: reports unique id# for actual runtime data representation **/
        virtual int32_t _typeseq() const = 0;

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

        virtual int32_t _typeseq() const final override { return s_typeseq; }

        virtual double xcoord(void * data) const final override { return Impl::xcoord(*(DRepr*)data); }
        virtual double ycoord(void * data) const final override { return Impl::ycoord(*(DRepr*)data); }
        virtual double argument(void * data) const final override { return Impl::argument(*(DRepr*)data); }
        virtual double magnitude(void * data) const final override { return Impl::magnitude(*(DRepr*)data); }

        virtual void destruct_data(void * data) const final override { Impl::destruct_data(*(DRepr*)data); }

        static int32_t s_typeseq;
        static bool _valid;
    };

    template <typename DRepr>
    int32_t
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
        virtual int32_t _typeseq() const final override { return s_typeseq; }

        virtual double xcoord(void *) const final override { assert(false); return 0.0; }
        virtual double ycoord(void *) const final override { assert(false); return 0.0; }
        virtual double argument(void *) const final override { assert(false); return 0.0; }
        virtual double magnitude(void *) const final override { assert(false); return 0.0; }

        virtual void destruct_data(void *) const final override { assert(false); }

        static int32_t s_typeseq;
        static bool _valid;
    };

    int32_t
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

        int32_t _typeseq() const { return Object::iface()->_typeseq(); }
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
            auto z1o = with_facet<AComplex>(&z1);

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

            *z1o = z1;

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
    }
}

/* end objectmodel.test.cpp */
