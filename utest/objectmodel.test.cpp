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
 *                             = RoutingFor<AComplex,OUniqueBox..>
 *                                      ^
 *                                      |
 *                             ubox<AComplex,
 *                                  DPolarCoords>
 *
 *  AComplex:              abstract interface
 *  DPolarCoords:          passive representation
 *  IComplex_DPolarCoords: implement AComplex interface for representation DPolarCoords
 *
 *  OUniqueBox<AComplex,DPolarCoords>:
 *                         a self-sufficient object, associating
 *                         interface AComplex with representation DPolarCoords
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
                double _xcoord(Repr *) const;
                double _ycoord(Repr *) const;
                double _argument(Repr *) const;
                double _magnitude(Repr *) const;

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
            using struct IComplex_DPolarCoords = IComplex_Specific<DPolarCoords>;

            template <>
            IComplex_Specific<DPolarCoords>::_xcoord(DPolarCoords * data) const {
                return data->mag_ * std::cos(data->arg_);
            };

            template <>
            IComplex_Specific<DPolarCoords>::_ycoord(DPolarCoords * data) const {
                return data->mag_ * std::sin(data->arg_);
            };

            template <>
            IComplex_Specific<DPolarCoords>::_argument(DPolarCoords * data) const {
                return data->arg_;
            }

            template <>
            IComplex_Specific<DPolarCoords>::_magnitude(DPolarCoords * data) const {
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
            using struct IComplex_DRectCoords = IComplex_Specific<DRectCoords>;

            template <>
            IComplex_Specific<DRectCoords>::_xcoord(DRectCoords * data) const {
                return data->mag_ * std::cos(data->arg_);
            };

            template <>
            IComplex_Specific<DRectCoords>::_ycoord(DRectCoords * data) const {
                return data->mag_ * std::sin(data->arg_);
            };

            template <>
            IComplex_Specific<DRectCoords>::_argument(DRectCoords * data) const {
                return data->arg_;
            }

            template <>
            IComplex_Specific<DRectCoords>::_magnitude(DRectCoords * data) const {
                return data->mag_;
            }

            template <>
            struct ISpecificFor<AComplex, DRectCoords> {
                using ImplType = IComplex_Specific<DRectCoords>;
            };

            template <>
            struct ISpecificFor<AComplex, DRectCoords> {
                using ImplType = IComplex_Specific<DRectCoords>;
            };

            // ----- box with unique pointer -----

            /** u for unique, b for box. Using lowercase for unobtrusiveness,
             *  so that in ub<MyType>, MyType is naturally emphasized
             *
             *  @tparam ISpecific will be a specific interface,
             *  such as ISpecificFor<AComplex, DRectCoords>
             *
             *  Example:
             *    OUniqueBox<AComplex, DRectCoords> z1 = ..;
             *    z1._xcoord(z1.data());
             **/
            template <typename AInterface, typename Data>
            struct OUniqueBox : ISpecificFor<AInterface, Data>::typename ImplType {
                Data * data() const { return data_.get(); }

                up<Data> data_;
            };

            template <typename Object>
            struct RComplex : public Object {
                double xcoord() const { return _xcoord(data()); }
                double ycoord() const { return _ycoord(data()); }
                double argument() const { return _argument(data()); }
                double magnitude() const { return _magnitude(data()); }
            }

            template <typename AInterface, typename Object>
            struct RoutingFor;

            template <typename Object>
            struct RoutingFor<AComplex, Object> {
                using RoutingType = RComplex<Object>;
            };

            /** boxed object, held by unique pointer
             *
             *  Example:
             *    ubox<AComplex, DRectCoords> z1 = ..;
             *    z1.xcoord();
             **/
            template <typename AInterface, typename Data>
            struct ubox : public RoutingFor<AInterface, Data>::typename RoutingType {
            }
        }
    }
}
