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
 *             ^                        ^
 *             |                        |
 *            ...             OUniqueBox
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
#include <cstring>

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

                virtual void destruct(void * data) const = 0;
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

                virtual void destruct(void *) const final override { assert(false); }
            };

            template <typename Repr>
            struct IComplex_Specific : public AComplex {
                static double _xcoord(Repr *);
                static double _ycoord(Repr *);
                static double _argument(Repr *);
                static double _magnitude(Repr *);
                static void _destruct(Repr *);

                virtual double xcoord(void * data) const final override { return _xcoord((Repr*)data); }
                virtual double ycoord(void * data) const final override { return _ycoord((Repr*)data); }
                virtual double argument(void * data) const final override { return _argument((Repr*)data); }
                virtual double magnitude(void * data) const final override { return _magnitude((Repr*)data); }
                virtual void destruct(void * data) const final override { _destruct((Repr*)data); }
            };

            // ----- Placeholder for opaque data -----

            // Placeholder used for template specialization

            struct DOpaquePlaceholder {};

            using IComplex_DOpaquePlaceholder = IComplex_Any;

            template <>
            struct ISpecificFor<AComplex, DOpaquePlaceholder> {
                using ImplType = IComplex_Any;
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
            void
            IComplex_Specific<DPolarCoords>::_destruct(DPolarCoords *) {
                /*trivial*/
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
            void
            IComplex_Specific<DRectCoords>::_destruct(DRectCoords * /*data*/) {
                /*trivial*/
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

            // ----- polymorphic box -----

            /**
             *  Unqiuely-owned instance with runtime polymorphism.
             *
             *  Unlike OUniqueBox<AInterface, ..> can use for variant data
             *  without additional overhead. Tradeoff is that avoiding such
             *  overhead excludes std::unique_ptr.
             *
             *  We're going to instead rely on AInterface providing a destruct() method,
             *  so in practice get the deleter from interface state.
             *
             *  Possibly means we need all abstract interfaces to share a common base
             **/
            template <typename AInterface, typename Data = DOpaquePlaceholder>
            struct OUniqueAny : ISpecificFor<AInterface, Data>::ImplType {
                /* note: Data can be void here */
                using DataType = Data;
                using DataBox = Data*;

                explicit OUniqueAny() {}
                /* unsatisfactory b/c doesn't enforce that @p d is heap-allocated */
                explicit OUniqueAny(DataBox d) : data_{std::move(d)} {}

                ~OUniqueAny() {
                    if (data_ != nullptr) {
                        this->destruct(data_);
                        delete data_;
                        this->data_ = nullptr;
                    }
                }

                /** note: load-bearing for routing classes such as RComplex<OUniqueAny> **/
                Data * data() const { return data_; }

                DataBox data_ = nullptr;
            };

            // ----- Router; RFoo pairs with AFoo -----

            template <typename Object>
            struct RComplex : public Object {
                RComplex() {}
                RComplex(Object::DataBox data) : Object{std::move(data)} {}

                double xcoord() const { return Object::xcoord(Object::data()); }
                double ycoord() const { return Object::ycoord(Object::data()); }
                double argument() const { return Object::argument(Object::data()); }
                double magnitude() const { return Object::magnitude(Object::data()); }
            };

            template <typename AInterface, typename Object>
            struct RoutingFor;

            template <typename Object>
            struct RoutingFor<AComplex, Object> {
                using RoutingType = RComplex<Object>;
            };

            template <typename AInterface, typename Object>
            using RoutingType = RoutingFor<AComplex, Object>::RoutingType;

            // ----- unique box; coordinates with OUniqueBox -----

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

            // ----- unique any; coordinates with OUniqueAny -----

            /** boxed object, held by unique-pointer equiavelent.
             *
             *  Example:
             *    std::unique_ptr<DRectCoords> z1_in
             *      = std::make_unique<DRectCoords>(1.0, 0.0):
             *    uany<AComplex> z1{z1_in.release()};
             *    z1.xcoord();
             *
             *
             *            +-----+             +-----------------+
             *  Interface |   x-------------->| vtable for      |
             *            +-----+             | some descendant |
             *       Data |   x--------\      | of AInterface   |
             *            +-----+      |      |                 |
             *                         |      +-----------------+
             *                         |
             *                         |      +--------------+
             *                         \----->| data :: Repr |
             *                                +--------------+
             *
             *  Binary representaiton of unay<AInterface, Data>
             *  is compatible for different values of @tparam Data
             *  as long as vtable pointer moves along with data pointer.
             *
             *  In particular binary representation for
             *  uany<AInterface,D> is as if it inherited uany<AInterface>
             *  (even though it does not as far as compiler is concerned)
             *
             *  This is load-bearing for @ref move2any see below
             **/
            template <typename AInterface, typename Data = DOpaquePlaceholder>
            struct uany : public RoutingType<AComplex, OUniqueAny<AComplex, Data>> {
                using Super = RoutingType<AComplex, OUniqueAny<AComplex, Data>>;

                uany() {}
                explicit uany(Super::DataBox d) : Super(d) {}

                /** copy contents of this instance into *dest.
                 **/
                void move2any(uany<AInterface> * dest) {
                    static_assert(sizeof(uany<AInterface>)
                                  == sizeof(uany<AInterface, Data>));

                    ::memcpy((void*)dest, (void*)this, sizeof(uany<AInterface>));
                    // this is almost right. But doesn't copy vtable pointer
                    //*dest = *(reinterpret_cast<uany<AInterface>*>(this));
                    this->data_ = nullptr;
                }

                /** move constructor from a different representation.
                 *  allowed given:
                 *  - same abstract interface
                 *  - same strategy (unique / refcounted / ..)
                 **/
                template <typename Data2>
                uany(uany<AInterface, Data2> && other)
                    requires (std::is_same_v<Data, DOpaquePlaceholder>
                              || std::is_convertible_v<Data2*, Data>)
                    : Super()
                {
                    static_assert(sizeof(uany<AInterface, Data2>)
                                  == sizeof(uany<AInterface, Data>));

                    other.move2any(this);

                    assert(other.data_ = nullptr);
                }
            };

        } /*namespace*/

        // ----- UNIT TESTS -----

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

        TEST_CASE("uany-1", "[objectmodel]")
        {
            /* default ctor */
            uany<AComplex> any;
        }

        TEST_CASE("uany-2", "[objectmodel]")
        {
            /* equivalent to ubox<AComplex,DRectCoords>, but impl doesn't use std::unique_ptr */
            uany<AComplex,DRectCoords> any{new DRectCoords{1.0, 0.0}};

            REQUIRE(any.xcoord() == 1.0);
            REQUIRE(any.ycoord() == 0.0);
            REQUIRE(any.argument() == 0.0);
            REQUIRE(any.magnitude() == 1.0);
        }

        TEST_CASE("uany-3", "[objectmodel]")
        {
            /* equivalent to ubox<AComplex,DRectCoords>, but impl doesn't use std::unique_ptr */
            uany<AComplex,DRectCoords> z1{new DRectCoords{1.0, 0.0}};

            DRectCoords * z1_data = z1.data();

            REQUIRE(z1.data() != nullptr);
            REQUIRE(z1.xcoord() == 1.0);

            /* can type-erase */
            uany<AComplex> z1_any;

            REQUIRE(z1_any.data() == nullptr);

            z1.move2any(&z1_any);

            /* z1 should be empty, it moved itself */
            REQUIRE(z1.data() == nullptr);
            REQUIRE((void*)z1_any.data() == (void*)z1_data);

            REQUIRE(z1_any.xcoord() == 1.0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end objectmodel.test.cp */
