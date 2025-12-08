/** @file objectmodel.test.cpp
 *
 *  @author: Roland Conybeare, Dec 2025
 *
 *  Testing rust-like split iface/data object model
 *  See xo-alloc2/README.md
 *
 *  Ingredients:
 *  1. abstract interface: all virtual methods. No assumptions about representation.
 *     No state (besides implict vtable pointer)
 *
 *  Rules:
 *  1. abstract interface must have no state besides implicit vtable pointer.
 *     This is a strongly-held principle, we're keeping data representation entirely
 *     separate
 *  2. representations as passive as possible. No getters. All public members.
 *     Exceptions to this principle:
 *     - ctors (including copy/move ctors, when needed)
 *     - dtors
 *
 *  Conventions:
 *  1. abstract interface start with letter A, e.g. AComplex
 *  2. representation struct names follow pattern DRepr, e.g. DPolar, DRect.
 *     Don't require "intended primary interface" in the name,
 *     since we're seeking ability to attach the same data to different interfaces
 *
 *  Example Class Diagram
 *
 *                                   AComplex
 *                                      ^
 *                                      |
 *             /------------------------+--------------------\
 *             |                        |                    |
 *    IComplex_DRectCoords   IComplex_DPolarCoords     IComplex_Any
 *     = IComplex_Specific    = IComplex_Specific
 *       <DRectCoords>          <DPolarCoords>
 *                                      ^
 *                                      |
 *                            OUniqueBox
 *                              <AComplex,DPolarCoords>
 *                                      ^
 *                                      |
 *                            RComplex<AComplex>
 *                             = RoutingFor<AComplex,OUniqueBox..>::RoutingType
 *                                      ^
 *                                      |
 *                             ubox<AComplex,
 *                                  DPolarCoords>
 *
 *  AComplex:              abstract interface.
 *                         explicit, type-erased, data pointer argument
 *                           virtual AComplex::xcoord(void * data)
 *
 *  DPolarCoords:          passive representation
 *
 *  IComplex_DPolarCoords: implement AComplex interface for representation DPolarCoords
 *                         static methods with typed data pointer argument
 *                           IComplex_DPolarCoords::xcoord(void * data)
 *                           IComplex_DPolarCoords::_xcoord(DPolarCoords * data)
 *
 *  OUniqueBox<AComplex,DPolarCoords>:
 *                         a self-sufficient object, associating
 *                         interface AComplex with representation DPolarCoords
 *                           OUniqueBox .data() method is DPolarCoord*
 *                         'impure' in the sense that it mixes code+data
 *
 *  RComplex<AComplex>:    convenience interface for OUniqueBox
 *
 *  ubox<AComplex,DPolarCOords>:
 *                         self-sufficent object with convenient interface
 *
 *  Application code will deal with ubox<AComplex,DPolarCoords>
 *
 *
 *
 *
 *
 *
 **/

#include <catch2/catch.hpp>
#include <cmath>
#include <cassert>

namespace xo {
    namespace ut {
        namespace {
            /** Associates an interface with an representation.
             *  Specialize to record such associations.
             **/

            template <typename Interface,
                      typename Data>
            struct ISpecificFor;

            /** type-erased implementation of AComplex, see below **/
            struct IComplex_Any;

            /** abstract interface for a complex number **/
            struct AComplex {
                using TypeErasedIface = IComplex_Any;

                virtual double xcoord(void * data) const = 0;
                virtual double ycoord(void * data) const = 0;
                virtual double argument(void * data) const = 0;
                virtual double magnitude(void * data) const = 0;
            };

            /** type-erased implementation of AComplex, for runtime polymorphism
             *  Usable by (and only by) overwriting with a typed implementation,
             *  such as IComplex_RectCoords or IComplex_PolarCoords.
             **/
            struct IComplex_Any : public AComplex {
                virtual double xcoord(void *) const final override { assert(false); return 0.0; }
                virtual double ycoord(void *) const final override { assert(false); return 0.0; }
                virtual double argument(void *) const final override { assert(false); return 0.0; }
                virtual double magnitude(void *) const final override { assert(false); return 0.0; }
            };

            template <typename Repr>
            struct IComplex_Specific : public AComplex {
                static double _xcoord(Repr *);
                static double _ycoord(Repr *);
                static double _argument(Repr *);
                static double _magnitude(Repr *);

                virtual double xcoord(void * data) const final override { return _xcoord((Repr*)data); }
                virtual double ycoord(void * data) const final override { return _ycoord((Repr*)data); }
                virtual double argument(void * data) const final override { return _argument((Repr*)data); }
                virtual double magnitude(void * data) const final override { return _magnitude((Repr*)data); }
            };

            // ----- Polar Coordinates -----

            /** complex number, represented using polar coordinates **/
            struct DPolarCoords {
                DPolarCoords(double arg, double mag) : arg_{arg}, mag_{mag} {}

                double arg_;
                double mag_;
            };

            /** implementation of AComplex interface with representation DPolarCoords **/
            using IComplex_DPolarCoords = IComplex_Specific<DPolarCoords>;

            template <>
            double
            IComplex_Specific<DPolarCoords>::_xcoord(DPolarCoords * data) {
                return data->mag_ * std::cos(data->arg_);
            };

            template <>
            double IComplex_Specific<DPolarCoords>::_ycoord(DPolarCoords * data) {
                return data->mag_ * std::sin(data->arg_);
            };

            template <>
            double
            IComplex_Specific<DPolarCoords>::_argument(DPolarCoords * data) {
                return data->arg_;
            }

            template <>
            double
            IComplex_Specific<DPolarCoords>::_magnitude(DPolarCoords * data) {
                return data->mag_;
            }

            template <>
            struct ISpecificFor<AComplex, DPolarCoords> {
                using ImplType = IComplex_Specific<DPolarCoords>;
            };

            // ----- Rectangular Coordinates -----

            /** complex number, represented using rectangular coordinates **/
            struct DRectCoords {
                DRectCoords(double x, double y) : x_{x}, y_{y} {}

                double x_;
                double y_;
            };

            /** implementation of AComplex interface with representation DRectCoords **/
            using IComplex_DRectCoords = IComplex_Specific<DRectCoords>;

            template <>
            double
            IComplex_Specific<DRectCoords>::_xcoord(DRectCoords * data) {
                return data->x_;
            };

            template <>
            double
            IComplex_Specific<DRectCoords>::_ycoord(DRectCoords * data) {
                return data->y_;
            };

            template <>
            double
            IComplex_Specific<DRectCoords>::_argument(DRectCoords * data) {
                return std::atan(data->y_ / data->x_);
            }

            template <>
            double
            IComplex_Specific<DRectCoords>::_magnitude(DRectCoords * data) {
                double x = data->x_;
                double y = data->y_;

                return std::sqrt(x*x + y*y);
            }

            template <>
            struct ISpecificFor<AComplex, DRectCoords> {
                using ImplType = IComplex_Specific<DRectCoords>;
            };

            // ----- box with unique pointer -----

            /**
             *  Creates a 'classic object-oriented'
             *  instance that has both interface+data.
             *
             *  OUniqueBox uses a unique_ptr to hold data,
             *  so lifetime ends (unless moved) when this OUniqueBox
             *  goes out of scope
             *
             *  policy:
             *  In our object model, these are not intended to be used
             *  for state; instead create them just-in-time.
             *
             *
             *  @tparam ISpecific will be a specific interface,
             *  such as ISpecificFor<AComplex, DRectCoords>
             *
             *  Example:
             *    OUniqueBox<AComplex, DRectCoords> z1 = ..;
             *    z1._xcoord(z1.data());
             **/
            template <typename AInterface, typename Data>
            struct OUniqueBox : ISpecificFor<AInterface, Data>::ImplType {
                using DataType = Data;
                using DataBox = std::unique_ptr<Data>;

                explicit OUniqueBox(DataBox d) : data_{std::move(d)} {}

                Data * data() const { return data_.get(); }

                DataBox data_;
            };

            template <typename Object>
            struct RComplex : public Object {
                RComplex(Object::DataBox data) : Object{std::move(data)} {}

                double xcoord() const { return Object::_xcoord(Object::data()); }
                double ycoord() const { return Object::_ycoord(Object::data()); }
                double argument() const { return Object::_argument(Object::data()); }
                double magnitude() const { return Object::_magnitude(Object::data()); }
            };

            template <typename AInterface, typename Object>
            struct RoutingFor;

            template <typename Object>
            struct RoutingFor<AComplex, Object> {
                using RoutingType = RComplex<Object>;
            };

            template <typename AInterface, typename Object>
            using RoutingType = RoutingFor<AComplex, Object>::RoutingType;

            /** boxed object, held by unique pointer
             *
             *  Example:
             *    ubox<AComplex, DRectCoords> z1 = ..;
             *    z1.xcoord();
             **/
            template <typename AInterface, typename Data>
            struct ubox : public RoutingType<AComplex, OUniqueBox<AComplex, Data>> {
                using Super = RoutingType<AComplex, OUniqueBox<AComplex, Data>>;

                explicit ubox(Super::DataBox d) : Super{std::move(d)} {}
            };
        } /*namespace*/

        TEST_CASE("objectmodel-specific-1", "[objectmodel]")
        {
            /* arg=0, mag=1 -> 1+0i */
            DPolarCoords polar{0.0, 1.0};
            IComplex_Specific<DPolarCoords> polar_iface;

            REQUIRE(polar_iface._xcoord(&polar) == 1.0);
            REQUIRE(polar_iface._ycoord(&polar) == 0.0);
            REQUIRE(polar_iface._argument(&polar) == 0.0);
            REQUIRE(polar_iface._magnitude(&polar) == 1.0);
        }

        TEST_CASE("objectmodel-specific-2", "[objectmodel]")
        {
            /* arg=0, mag=1 -> 1+0i */
            DRectCoords rect{1.0, 0.0};
            IComplex_Specific<DRectCoords> rect_iface;

            REQUIRE(rect_iface._xcoord(&rect) == 1.0);
            REQUIRE(rect_iface._ycoord(&rect) == 0.0);
            REQUIRE(rect_iface._argument(&rect) == 0.0);
            REQUIRE(rect_iface._magnitude(&rect) == 1.0);
        }

        TEST_CASE("uniquebox-1", "[objectmodel]")
        {
            OUniqueBox<AComplex, DPolarCoords> box
                {std::make_unique<DPolarCoords>(0.0, 1.0)};

            REQUIRE(box.xcoord(box.data()) == 1.0);
            REQUIRE(box.ycoord(box.data()) == 0.0);
            REQUIRE(box.argument(box.data()) == 0.0);
            REQUIRE(box.magnitude(box.data()) == 1.0);
        }

        TEST_CASE("router-1", "[objectmodel]")
        {
            using Object = OUniqueBox<AComplex, DPolarCoords>;

            RComplex<Object> box{std::make_unique<DPolarCoords>(0.0, 1.0)};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("routing-type-1", "[objectmodel]")
        {
            using Object = OUniqueBox<AComplex, DPolarCoords>;

            RoutingType<AComplex, Object> box{std::make_unique<DPolarCoords>(0.0, 1.0)};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("ubox-1", "[objectmodel]")
        {
            ubox<AComplex,DPolarCoords> box{std::make_unique<DPolarCoords>(0.0, 1.0)};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("ubox-2", "[objectmodel]")
        {
            ubox<AComplex,DRectCoords> box{std::make_unique<DRectCoords>(1.0, 0.0)};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end objectmodel.test.cp */
