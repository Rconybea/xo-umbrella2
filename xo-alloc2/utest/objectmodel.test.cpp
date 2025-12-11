/** @file objectmodel.test.cpp
 *
 *  @author: Roland Conybeare, Dec 2025
 *
 *  Testing rust-like traits (split iface/data) object model.
 *  Analogous to:
 *  - rust traits
 *  - haskell type classes
 *  - go interfaces
 *
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
 *  2. representation struct names start with letter D, e.g. DPolar, DRect.
 *     Don't require "intended primary interface" in the name,
 *     since we're seeking ability to attach the same data to different interfaces
 *  3. implementations start with letter I. They concatenate abstract interface name
 *     and representation name, e.g. IComplex_PolarCoords
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
 **/

#include <catch2/catch.hpp>
#include <cmath>
#include <cassert>
#include <cstring>

namespace xo {
    namespace ut {
        namespace {
            struct PlaceholderAbstractInterface {
                virtual double foo(void * data) const = 0;
            };

            static_assert(sizeof(PlaceholderAbstractInterface) == sizeof(void*));

            /** Concept: abstract interface requirements
             *  Use: when inheriting an abstract interface
             *  (see also valid_abstract_interface() below)
             **/
            template <typename T>
            concept abstract_interface = requires {
                std::is_abstract_v<T>,
                std::is_polymorphic_v<T>;
                /** require no state, just a single vtable pointer **/
                sizeof(T) == sizeof(PlaceholderAbstractInterface);
                !std::has_virtual_destructor_v<T>;
                std::is_trivially_destructible_v<T>;
            };

            /** For example ISpecific = IComplex_DPolarCoords
             **/
            template <typename AInterface, typename ISpecific>
            concept implements_interface = requires {
                std::is_base_of_v<AInterface, ISpecific>;
                std::is_default_constructible_v<ISpecific>;
                std::is_standard_layout_v<ISpecific>;
                /** require no additional state **/
                sizeof(ISpecific) == sizeof(AInterface);
            };

            /** Router delivers data to interface implementation **/
            template <typename Object, typename Router>
            concept provides_router = requires {
                std::is_base_of_v<Object, Router>;
                sizeof(Router) == sizeof(Object);
            };

            /** Use: when defining an abstract interface AMyInterface
             *
             *    struct AMyInterface {
             *        virtual void foo(void * data) const = 0;
             *    };
             *
             *    static_assert(valid_abstract_interface<AMyInterface>());
             *
             **/
            template <typename T>
            consteval bool valid_abstract_interface()
            {
                static_assert(std::is_abstract_v<T>,
                              "Abstract interface expected to have all-abstract methods");
                static_assert(std::is_polymorphic_v<T>,
                              "Abstract interface expected to have vtable");
                static_assert(sizeof(T) == sizeof(PlaceholderAbstractInterface),
                              "Abstract interface expected to have no state except for a single vtable pointer");
                static_assert(!std::has_virtual_destructor_v<T>,
                              "Abstract interface does not benefit from virtual dtor since no state");
                static_assert(std::is_trivially_destructible_v<T>,
                              "Abstract interface expected to have trivial dtor since no state");
                return true;
            };

            template <typename AInterface, typename ISpecific>
            consteval bool valid_interface_implementation()
            requires (valid_abstract_interface<AInterface>())
            {
                static_assert(std::is_base_of_v<AInterface, ISpecific>,
                              "Interface implementation must inherit abstract interface");
                static_assert(std::is_default_constructible_v<ISpecific>,
                              "Interface implementation must be default-constructible");
                static_assert(sizeof(ISpecific) == sizeof(AInterface),
                              "Interface implementation may no introduce state");
                static_assert(!std::has_virtual_destructor_v<ISpecific>,
                              "Interface implementation may does not benefit from virtual dtor since no state");
                static_assert(std::is_trivially_destructible_v<ISpecific>,
                              "Interface implementation expected to have trivial dtor since no state");

                // don't need this test, it's covered by sizeof check
                //static_assert(std::is_pointer_interconvertible_base_of_v<AInterface, ISpecific>,
                //              "Interface implementation must directly inherit interface (no base offset)");

                return true;
            };

            template <typename OObject>
            consteval bool valid_object_traits()
            {
                static_assert(requires { typename OObject::AbstractInterface; },
                              "Object type must provide typename Object::AbstractInterface");
                static_assert(requires { typename OObject::ISpecific; },
                              "Object type must provide typename Object::ISpecific");
                static_assert(requires { typename OObject::DataType; },
                              "Object type must provide typename Object::DataType");
                static_assert(valid_interface_implementation<OObject::AbstractInterface, OObject::ISpecific>,
                              "Object::ISpecific must implement Object::AbstractInterface");
                static_assert(std::is_standard_layout_v<OObject>,
                              "Object must have standard layout, i.e. no virtual methods. Virtual methods belong in OObject::AbstractInterface");
                static_assert(requires(const OObject & obj) { { obj.iface() } -> std::convertible_to<const typename OObject::AbstractInterface*>; },
                              "Object must have non-virtual method iface() returning const Object::AbstractInterface");
                static_assert(requires(const OObject & obj) { { obj.data() } -> std::convertible_to<typename OObject::DataType*>; },
                              "Object must have non-virtual method data() returning Object::DataType*");

                return true;
            }

            template <typename RRouter>
            consteval bool valid_object_router()
            {
                static_assert(requires { typename RRouter::ObjectType; },
                              "Router type must provide typename Router::ObjectType");
                static_assert(valid_object_traits<RRouter::ObjectType>,
                              "Router::ObjectType must satisfy objectmodel traits");
                static_assert(std::is_standard_layout_v<RRouter>,
                              "Router must have standard laayout, i.e. no virtual methods. Virtual methods belong in OObject::AbstractInterface*>");
                return true;
            };

            // ----------------------------------------------------------------

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

                virtual void destruct_data(void * data) const = 0;

            private:
                static bool _valid;
            };

            bool
            AComplex::_valid = valid_abstract_interface<AComplex>();

            // ----------------------------------------------------------------

            /** type-erased implementation of AComplex, for runtime polymorphism
             *  Usable by (and only by) overwriting with a typed implementation,
             *  such as IComplex_RectCoords or IComplex_PolarCoords.
             **/
            struct IComplex_Any : public AComplex {
                virtual double xcoord(void *) const final override { assert(false); return 0.0; }
                virtual double ycoord(void *) const final override { assert(false); return 0.0; }
                virtual double argument(void *) const final override { assert(false); return 0.0; }
                virtual double magnitude(void *) const final override { assert(false); return 0.0; }

                virtual void destruct_data(void *) const final override { assert(false); }

            private:
                static bool _valid;
            };

            bool
            IComplex_Any::_valid = valid_interface_implementation<AComplex, IComplex_Any>();

            // ----------------------------------------------------------------

            template <typename Repr>
            struct IComplex_Specific : public AComplex {
                static double _xcoord(Repr *);
                static double _ycoord(Repr *);
                static double _argument(Repr *);
                static double _magnitude(Repr *);
                static void _destruct_data(Repr *);

                virtual double xcoord(void * data) const final override { return _xcoord((Repr*)data); }
                virtual double ycoord(void * data) const final override { return _ycoord((Repr*)data); }
                virtual double argument(void * data) const final override { return _argument((Repr*)data); }
                virtual double magnitude(void * data) const final override { return _magnitude((Repr*)data); }
                virtual void destruct_data(void * data) const final override { _destruct_data((Repr*)data); }

            public:
                static bool _valid;
            };

            template <typename Repr>
            bool
            IComplex_Specific<Repr>::_valid = valid_interface_implementation<AComplex, IComplex_Specific>;

            // ----- Placeholder for opaque data -----

            // Placeholder used for template specialization

            struct DOpaquePlaceholder {};

            using IComplex_DOpaquePlaceholder = IComplex_Any;

            template <>
            struct ISpecificFor<AComplex, DOpaquePlaceholder> {
                using ImplType = IComplex_Any;
            };

            // ----- Representation: Polar Coordinates -----

            /** complex number, represented using polar coordinates **/
            struct DPolarCoords {
                DPolarCoords(double arg, double mag) : arg_{arg}, mag_{mag} {}

                double arg_;
                double mag_;
            };

            // ----- AComplex for DPolarCoords -----

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
            IComplex_Specific<DPolarCoords>::_destruct_data(DPolarCoords * data) {
                data->~DPolarCoords();
            }

            template <>
            struct ISpecificFor<AComplex, DPolarCoords> {
                using ImplType = IComplex_Specific<DPolarCoords>;
            };

            // ----- Representation: Rectangular Coordinates -----

            /** complex number, represented using rectangular coordinates **/
            struct DRectCoords {
                DRectCoords(double x, double y) : x_{x}, y_{y} {}

                double x_;
                double y_;
            };

            // ----- AComplex for DRectCoords -----

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
            IComplex_Specific<DRectCoords>::_destruct_data(DRectCoords * data) {
                data->~DRectCoords();
            }

            template <>
            struct ISpecificFor<AComplex, DRectCoords> {
                using ImplType = IComplex_Specific<DRectCoords>;
            };

            // ----- polymorphic box -----

            /**
             *  Unqiuely-owned instance with runtime polymorphism.
             *
             *  Unlike OUniqueBox<AInterface, ..> can use for variant data
             *  without additional overhead. Tradeoff is that avoiding such
             *  overhead excludes std::unique_ptr.
             *
             *  We're going to instead rely on AInterface providing a destruct_data() method,
             *  so in practice get the deleter from interface state.
             *
             *  Possibly means we need all abstract interfaces to share a common base
             *
             *  Remarks:
             *  - when @tparam Data is supplied
             **/
            template <typename AInterface, typename Data = DOpaquePlaceholder>
            struct OUniqueBox {
                using AbstractInterface = AInterface;
                using ISpecific = ISpecificFor<AInterface, Data>::ImplType;
                /* note: Data can be void here */
                using DataType = Data;
                using DataBox = Data*;

                explicit OUniqueBox() {}
                /* unsatisfactory b/c doesn't enforce that @p d is heap-allocated */
                explicit OUniqueBox(DataBox d) : data_{std::move(d)} {}

                ~OUniqueBox() {
                    if (data_ != nullptr) {
                        this->iface()->destruct_data(data_);
                        delete data_;
                        this->data_ = nullptr;
                    }
                }

                const AInterface * iface() const
                   requires std::is_same_v<Data, DOpaquePlaceholder>
                {
                    return std::launder(&iface_);
                }

                const AInterface * iface() const
                  requires (!std::is_same_v<Data, DOpaquePlaceholder>)
                {
                    return &iface_;
                }

                /** note: would prefer this to be constexpr, but not simple asof gcc 14.3 **/
                static bool _valid;

                /** note: load-bearing for routing classes such as RComplex<OUniqueBox> **/
                Data * data() const { return data_; }

                ISpecific iface_;
                DataBox data_ = nullptr;
            };

            template <typename AInterface, typename Data>
            bool
            OUniqueBox<AInterface, Data>::_valid = valid_object_traits<OUniqueBox>();

            // ----- Router; RFoo pairs with AFoo -----

            /**  For example, inherit OUniqueBox<AComplex>
             **/
            template <typename Object>
            struct RComplex : public Object {
                using ObjectType = Object;

                RComplex() {}
                RComplex(Object::DataBox data) : Object{std::move(data)} {}

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

            template <typename AInterface, typename Object>
            requires abstract_interface<AInterface>
            struct RoutingFor;

            template <typename Object>
            struct RoutingFor<AComplex, Object> {
                using RoutingType = RComplex<Object>;
            };

            template <typename AInterface, typename Object>
            using RoutingType = RoutingFor<AComplex, Object>::RoutingType;

            // ----- unique any; coordinates with OUniqueBox -----

            /** boxed object, held by unique-pointer equivalent.
             *  - With default Data argument:
             *    type-erased polymorphic container
             *  - with specific Data argument:
             *    typed container. Trivially de-virtualizable
             *
             *  Example:
             *    std::unique_ptr<DRectCoords> z1_in
             *      = std::make_unique<DRectCoords>(1.0, 0.0):
             *    ubox<AComplex> z1{z1_in.release()};
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
             *  Binary representation of unay<AInterface, Data>
             *  is compatible for different values of @tparam Data
             *  as long as vtable pointer moves along with data pointer.
             *
             *  In particular binary representation for
             *  ubox<AInterface,D> is as if it inherited ubox<AInterface>
             *  (even though it does not as far as compiler is concerned)
             *
             *  This is load-bearing for @ref move2any see below
             **/
            template <typename AInterface, typename Data = DOpaquePlaceholder>
            struct ubox : public RoutingType<AComplex, OUniqueBox<AComplex, Data>> {
                using Super = RoutingType<AComplex, OUniqueBox<AComplex, Data>>;

                ubox() {}
                explicit ubox(Super::DataBox d) : Super(d) {}

                /** copy contents of this instance into *dest.
                 **/
                void move2any(ubox<AInterface> * dest) {
                    static_assert(sizeof(ubox<AInterface>)
                                  == sizeof(ubox<AInterface, Data>));

                    ::memcpy((void*)dest, (void*)this, sizeof(ubox<AInterface>));
                    // this is almost right. But would not copy vtable pointer
                    //*dest = *(reinterpret_cast<ubox<AInterface>*>(this));
                    this->data_ = nullptr;
                }

                /** move constructor from a different representation.
                 *  allowed given:
                 *  - same abstract interface
                 *  - same strategy (unique / refcounted / ..)
                 **/
                template <typename Data2>
                ubox(ubox<AInterface, Data2> && other)
                    requires (std::is_same_v<Data, DOpaquePlaceholder>
                              || std::is_convertible_v<Data2*, Data>)
                    : Super()
                {
                    static_assert(sizeof(ubox<AInterface, Data2>)
                                  == sizeof(ubox<AInterface, Data>));

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
            auto tmp = std::make_unique<DPolarCoords>(0.0, 1.0);
            OUniqueBox<AComplex, DPolarCoords> box{tmp.release()};

            REQUIRE(box.iface()->xcoord(box.data()) == 1.0);
            REQUIRE(box.iface()->ycoord(box.data()) == 0.0);
            REQUIRE(box.iface()->argument(box.data()) == 0.0);
            REQUIRE(box.iface()->magnitude(box.data()) == 1.0);
        }

        TEST_CASE("router-1", "[objectmodel]")
        {
            using Object = OUniqueBox<AComplex, DPolarCoords>;
            auto tmp = std::make_unique<DPolarCoords>(0.0, 1.0);

            RComplex<Object> box{tmp.release()};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("routing-type-1", "[objectmodel]")
        {
            using Object = OUniqueBox<AComplex, DPolarCoords>;
            auto tmp = std::make_unique<DPolarCoords>(0.0, 1.0);

            RoutingType<AComplex, Object> box{tmp.release()};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("ubox-1", "[objectmodel]")
        {
            auto tmp = std::make_unique<DPolarCoords>(0.0, 1.0);
            ubox<AComplex,DPolarCoords> box{tmp.release()};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("ubox-2", "[objectmodel]")
        {
            auto tmp = std::make_unique<DRectCoords>(1.0, 0.0);
            ubox<AComplex,DRectCoords> box{tmp.release()};

            REQUIRE(box.xcoord() == 1.0);
            REQUIRE(box.ycoord() == 0.0);
            REQUIRE(box.argument() == 0.0);
            REQUIRE(box.magnitude() == 1.0);
        }

        TEST_CASE("ubox-any-1", "[objectmodel]")
        {
            /* default ctor */
            ubox<AComplex> any;
        }

        TEST_CASE("ubox-any-2", "[objectmodel]")
        {
            /* equivalent to ubox<AComplex,DRectCoords>, but impl doesn't use std::unique_ptr */
            ubox<AComplex,DRectCoords> any{new DRectCoords{1.0, 0.0}};

            REQUIRE(any.xcoord() == 1.0);
            REQUIRE(any.ycoord() == 0.0);
            REQUIRE(any.argument() == 0.0);
            REQUIRE(any.magnitude() == 1.0);
        }

        TEST_CASE("ubox-any-3", "[objectmodel]")
        {
            /* equivalent to ubox<AComplex,DRectCoords>, but impl doesn't use std::unique_ptr */
            ubox<AComplex,DRectCoords> z1{new DRectCoords{1.0, 0.0}};

            DRectCoords * z1_data = z1.data();

            REQUIRE(z1.data() != nullptr);
            REQUIRE(z1.xcoord() == 1.0);

            /* can type-erase */
            ubox<AComplex> z1_any;

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
