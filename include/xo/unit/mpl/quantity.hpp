/* @file quantity.hpp */

#pragma once

#include "quantity_concept.hpp"
#include "unit.hpp"
#include "detail/promoter.hpp"
//#include "xo/reflect/Reflect.hpp"
//#include "xo/indentlog/scope.hpp"

namespace xo {
    namespace unit {
        // ----- quantity -----

        /** @class quantity
         *
         *  @brief represents a scalar quantity; enforces dimensional consistency at compile time.
         *
         * - @p Unit is a type identifying dimension and scale attaching to this quantity.
         *   Unit must satisfy @c unit_concept<Unit>
         * - @p Repr is a type used to represent quantity values, scaled by @p Unit.
         *   Repr must satisfy @c numeric_concept<Repr>
         *
         * A quantity's run-time state consists of exactly one @p Repr
         * instance:
         *   @c sizeof(quantity<Unit, Repr>) == sizeof(Repr)
         **/
        template <typename Unit, typename Repr = double>
        class quantity {
        public:
            /** @defgroup mpl-quantity-traits **/
            ///@{
            /** @brief type capturing the units (and dimension) of this quantity **/
            using unit_type = Unit;
            /** @brief type used for representation of this quantity **/
            using repr_type = Repr;
            ///@}

            static_assert(unit_concept<Unit>);
            static_assert(numeric_concept<Repr>);
            /* non-unity compile-time scale factors can arise during unit conversion;
             * for example see method quantity::in_units_of()
             */
            static_assert(std::same_as< typename Unit::scalefactor_type, std::ratio<1> >);
            static_assert(std::same_as< typename Unit::canon_type, typename Unit::canon_type >);

        public:
            /** @defgroup mpl-quantity-ctors constructors
             **/
            ///@{
            constexpr quantity() = default;
            constexpr quantity(quantity const & x) = default;
            constexpr quantity(quantity && x) = default;
            ///@}

            /** @defgroup quantity-named-ctors named constructors
             **/
            ///@{
            /** @brief construct a unit quantity using @c unit_type
             *
             *  @code
             *  auto q = qty::milliseconds(17) / qty::kilometers(23.0);
             *  q::unit_quantity();   // 1ms.km^-1
             *  @endcode
             **/
            static constexpr quantity unit_quantity() { return quantity(1); }
            /** @brief promote representation to quantity.  Same as multiplying by Unit
             **/
            static constexpr auto promote(Repr x);
            ///@}

            /** @addtogroup mpl-quantity-traits **/
            ///@{

            /** @brief report this quantity's basis-power-unit type for a given basis dimension
             *
             *  Example:
             *  @code
             *  auto q = 1.0 / (qty::milliseconds(5) * qty::seconds(100.0));
             *  q.unit_cstr(); // "ms^-2"
             *
             *  using tmp = q.find_bpu_t<dim::time>;
             *
             *  tmp::c_native_dim;          // dim::time
             *  tmp::c_native_unit;         // native_unit_id::second
             *  tmp::scalefactor_type::num; // 1
             *  tmp::scalefactor_type::den; // 1000
             *  tmp::power_type::num;       // -2
             *  tmp::pwoer_type::den;       // 1
             *  @endcode
             **/
            template <dim BasisDim>
            using find_bpu_t = unit_find_bpu_t<unit_type, BasisDim>;

            /** @brief report this quantity's scalefactor type for given basis dimension **/
            template <dim BasisDim>
            using basis_scale_type = typename find_bpu_t<BasisDim>::scalefactor_type::type;

            ///@}

            /** @defgroup mpl-quantity-access-methods **/
            ///@{
            /** @brief get scale value (relative to unit) (@ref scale_) **/
            constexpr Repr scale() const { return scale_; }
            /** @brief abbreviation for this quantity's units
             *
             *  This string literal is constructed at compile-time by concatenating
             *  abbreviations for each basis-power-unit.
             *  For implementation see:
             *  * @c xo::unit::native_unit_abbrev_helper
             *    (in xo/unit/basis_unit.hpp) for each native dimension
             *  * @c xo::unit::scaled_native_unit_abbrev
             *    (in xo/unit/basis_unit.hpp) last-resort handling for scaled native dimensions
             *  * @c xo::unit::scaled_native_unit_abbrev
             *    (in xo/unit/unit.hpp) specializations for scaled native dimensions
             **/
            static constexpr char const * unit_cstr() { return unit_abbrev_v<unit_type>.c_str(); }
            ///@}

            /** @defgroup mpl-quantity-constants constants
             **/
            ///@{
            /** @brief report exponent of @p BasisDim in dimension of this quantity
             *
             *  For example:
             *  @code
             *  auto q = qty::milliseconds(5) * qty::seconds(1);
             *  int p1 = q.basis_power<dim::time>; // p1 == 2
             *  int p2 = q.basis_power<dim::mass>; // p2 == 0
             *  @endcode
             **/
            template <dim BasisDim, typename PowerRepr = int>
            static constexpr PowerRepr basis_power = from_ratio<PowerRepr,
                                                                typename find_bpu_t<BasisDim>::power_type>();
            ///@}

            /** @defgroup quantity-unit-conversion **/
            ///@{
            /** @brief convert to quantity representing the same amount, but changing units and perhaps representation.
             *
             *  These two expressions are equivalent:
             *  @code
             *  q.with_unit<units::millisecond>();
             *  quantity<units::millisecond, q::repr_type>(q);
             *  @endcode
             *
             **/
            template <typename Unit2, typename Repr2 = repr_type>
            constexpr quantity<Unit2, Repr2> with_unit() const { return *this; }

            /**
             * @brief produce quantity scaled according to @p BasisUnit2,  representing the same value as @c *this.
             *
             * For example:
             *
             * @code{.cpp}
             * auto q1 = 1.0 / minutes(1) * kilograms(2.5);         // q1 = 2.5kg.min^-1
             * auto q2 = q1.with_basis_unit<units::millisecond>();  // q2 in kg.ms^-1
             * @endcode
             *
             * Motivation is ability to chain rescaling to reach desired compound unit
             *
             * @code
             * auto q3 = q1.with_basis_unit<units::second>()
             *             .with_basis_unit<units::gram>();   // q3 in g.s^-1
             * @endcode
             **/
            template <typename BasisUnit2, typename Repr2 = repr_type>
            constexpr auto with_basis_unit() const {
                static_assert(basis_unit_concept<BasisUnit2>);

                using new_bpu_type = BasisUnit2::dim_type::front_type;
                using old_bpulist_type = unit_type::dim_type;
                using new_bpulist_type = di_replace_basis_scale<old_bpulist_type, new_bpu_type>::type;
                using new_unit_type = wrap_unit<std::ratio<1>, new_bpulist_type>;

#              ifdef NOT_USING_DEBUG
                using xo::reflect::Reflect;
                scope log(XO_DEBUG(true /*c_debug_flag*/));
                log && log(xtag("old_unit_type", Reflect::require<unit_type>()->canonical_name()));
                log && log(xtag("new_unit_type", Reflect::require<new_unit_type>()->canonical_name()));
#              endif

                return this->with_unit<new_unit_type, Repr2>();
            }

            /**
             * @brief express this quantity in the same units as @p q
             *
             * @pre @c *this and @p q must have the same dimension
             *
             * @param q  take units from @c q::unit_type, ignoring @c q.scale()
             * @return this amount, but expressed using the same units as @p q
             **/
            template <typename Quantity>
            auto with_units_from(Quantity q) const {
                return this->with_units<typename Quantity::unit_type>();
            }

            /**
             * @brief express this quantity in units of @p Unit2.
             *
             * @p Unit2 specifies new units
             * @p Repr2 specifies representation
             * @return this amount, but expressed as a multiple of @p Unit2
             **/
            template <typename Unit2, typename Repr2 = repr_type>
            auto with_units() const {
                Repr2 x = this->in_units_of<Unit2, Repr2>();

                return quantity<Unit2, Repr2>::promote(x);
            }

            /**
             * @brief compute scale with respect to @p Unit2
             *
             * @pre @c *this must have the same dimension as @p Unit2
             *
             * @p Unit2   rescale in terms of this unit.
             * @p Repr2   compute scale in this representation
             * @return scale to use for @c quantity<Unit2,Repr2> representing the same amount as @c *this.
             **/
            template <typename Unit2, typename Repr2 = repr_type>
            Repr2 in_units_of() const {
                // static_assert(dimension_of<Unit> == dimension_of<Unit2>);  // discard all the scaling values

                static_assert(same_dimension_v<Unit, Unit2>);

                using _convert_to_u2_type = unit_cartesian_product<Unit, unit_invert_t<Unit2>>;

                using exact_scalefactor_type = _convert_to_u2_type::exact_unit_type::scalefactor_type;
                constexpr double c_scalefactor_inexact = _convert_to_u2_type::c_scalefactor_inexact;

                // _convert_u2_type
                //  - scalefactor_type
                //  - dim_type
                //  - canon_type

                /* if _convert_u2_type isn't dimensionless,  then {Unit2, Unit} have different dimensions */

                return ((this->scale_ * c_scalefactor_inexact * exact_scalefactor_type::num) / exact_scalefactor_type::den);
            }

            /**
             * @brief convert to quantity with  representation @p Repr2
             *
             * @return a quantity representing the same amount as @c *this, but using representation @p Repr2
             **/
            template <typename Repr2>
            constexpr quantity<unit_type, Repr2> with_repr() const { return quantity<unit_type, Repr2>::promote(scale_); }
            ///@}


            /** @defgroup quantity-arithmeticsupport **/
            ///@{
            /**
             * @brief multiply this quantity *x* by another quantity *y*.
             *
             * Result will propagate dimension and units appropriately.
             * If *x* and *y* use conflicting scale factors for a dimension,
             * adopt scalefactor from *x*.
             *
             * note: result will be a dimensionless value (e.g. type @c double)
             * if units cancel.
             *
             * @pre @p Quantity2 must satisfy @c quantity_concept<Quantity2>
             *
             * @param y   multiply by this amount
             * @return x.multiply(y) returns amount representing x*y
             **/
            template <typename Quantity2>
            auto multiply(Quantity2 y) const {
                //constexpr bool c_debug_flag = false;
                //using Reflect = xo::reflect::Reflect;

                //scope log(XO_DEBUG(c_debug_flag));

                static_assert(quantity_concept<Quantity2>);

                /* unit: may have non-unit scalefactor_type */
                using unit_product_type = unit_cartesian_product<Unit, typename Quantity2::unit_type>;
                using exact_unit_type = unit_product_type::exact_unit_type;
                using norm_unit_type = normalize_unit_t<exact_unit_type>;

                using exact_scalefactor_type = exact_unit_type::scalefactor_type;
                constexpr double c_scalefactor_inexact = unit_product_type::c_scalefactor_inexact;

                using repr_type = std::common_type_t<repr_type, typename Quantity2::repr_type>;

                repr_type r_scale = ((scale() * y.scale() * c_scalefactor_inexact * exact_scalefactor_type::num)
                                     / exact_scalefactor_type::den);

#              ifdef NOT_USING_DEBUG
                log && log(xtag("unit_product_type", Reflect::require<unit_product_type>()->canonical_name()));
                log && log(xtag("exact_unit_type", Reflect::require<exact_unit_type>()->canonical_name()));
                log && log(xtag("norm_unit_type", Reflect::require<norm_unit_type>()->canonical_name()));
                log && log(xtag("exact_scalefactor_type", Reflect::require<exact_scalefactor_type>()->canonical_name()));
                log && log(xtag("c_scalefactor_inexact", c_scalefactor_inexact));
                log && log(xtag("repr_type", Reflect::require<repr_type>()->canonical_name()));
                log && log(xtag("repr_type", Reflect::require<repr_type>()->canonical_name()));
#              endif

                return quantity<norm_unit_type, repr_type>::promote(r_scale);
            }

            /**
             * @brief multiply this quantity *x* by another quantity *y*
             *
             * Result will propagate dimension and units appropriately.
             * If *x* and *y* use conflicting scale factors for a dimension,
             * adopt scalefactor from *x*.
             *
             * note: result will be a dimensionless value (e.g. type @c double)
             * if units cancel.
             *
             * @pre @p Quantity2 must satisfy @c quantity_concept<Quantity2>
             *
             * @param y   divide by this amount
             * @return x.divide(y) returns amount representing x/y
             **/
            template <typename Quantity2>
            auto divide(Quantity2 y) const {
                using unit_divide_type = unit_divide<Unit, typename Quantity2::unit_type>;
                using exact_unit_type = unit_divide_type::exact_unit_type;
                using norm_unit_type = normalize_unit_t<exact_unit_type>;

                using exact_scalefactor_type = exact_unit_type::scalefactor_type;
                constexpr double c_scalefactor_inexact = unit_divide_type::c_scalefactor_inexact;

                using repr_type = std::common_type_t<repr_type, typename Quantity2::repr_type>;

                repr_type r_scale = ((scale() * c_scalefactor_inexact * exact_scalefactor_type::num)
                                     / (y.scale() * exact_scalefactor_type::den));

#              ifdef NOT_USING_DEBUG
                using xo::reflect::Reflect;
                scope log(XO_DEBUG(true /*c_debug_flag*/));
                log && log(xtag("unit_divide_type", Reflect::require<unit_divide_type>()->canonical_name()));
                log && log(xtag("exact_unit_type", Reflect::require<exact_unit_type>()->canonical_name()));
                log && log(xtag("norm_unit_type", Reflect::require<norm_unit_type>()->canonical_name()));
                log && log(xtag("exact_scalefactor_type", Reflect::require<exact_scalefactor_type>()->canonical_name()));
                log && log(xtag("c_scalefactor_inexact", c_scalefactor_inexact));
                log && log(xtag("r_scale", r_scale));
                log && log(xtag("repr_type", Reflect::require<repr_type>()->canonical_name()));
#              endif

                return quantity<norm_unit_type, repr_type>::promote(r_scale);
            }

            // quantity operator*=()
            // quantity operator/=()

            /**
             *  @brief scale this quantity *x* by dimensionless amount @p y
             *
             *  @return quantity representing @c x*y
             **/
            template <typename Repr2>
            auto scale_by(Repr2 y) const {
                static_assert(!quantity_concept<Repr2>);

                using r_repr_type = std::common_type_t<repr_type, Repr2>;

                r_repr_type r_scale = this->scale_ * y;

                //std::cerr << "quantity::scale_by: scale=" << scale << ", repr_type=" << reflect::Reflect::require<repr_type>()->canonical_name() << std::endl;

                return quantity<unit_type, r_repr_type>::promote(r_scale);
            }

            /**
             *  @brief divide this quantity *x* by dimensionless amount @p y
             *
             *  @return quantity representing @c x/y
             **/
            template <typename Repr2>
            auto divide_by(Repr2 x) const {
                using r_repr_type = std::common_type_t<repr_type, Repr2>;

                r_repr_type r_scale = this->scale_ / x;

                return quantity<unit_type, r_repr_type>::promote(r_scale);
            }

            /**
             *  @brief divide dimensionless number @p x by this quantity @c y
             *
             *  @return quantity representing @c x/y
             **/
            template <typename Repr2>
            auto divide_into(Repr2 x) const {
                using r_unit_type = unit_invert_t<Unit>;
                using r_repr_type = std::common_type_t<repr_type, Repr2>;

                r_repr_type r_scale = ((x * r_unit_type::scalefactor_type::num)
                                       / (this->scale_ * r_unit_type::scalefactor_type::den));

                return quantity<r_unit_type, r_repr_type>::promote(r_scale);
            }
            ///@}

            /** @defgroup mpl-quantity-arithmetic **/
            ///@{
            /** @brief add quantity in-place
             *
             *  @pre @p y must have the same dimension as @c *this.
             *
             *  @param y  quantity to add
             *  @return this quantity after adding y
             **/
            template <typename Quantity2>
            quantity & operator+=(Quantity2 y) {
                static_assert(same_dimension_v<unit_type, typename Quantity2::unit_type>);

                /* relying on assignment that correctly converts-to-lhs-units */
                quantity y2 = y;

                this->scale_ += y2.scale();

                return *this;
            }

            /** @brief subtract quantity in-place
             *
             *  @pre @p y must have the same dimensions as @c *this
             *
             *  @param y  quantity to subtract
             *  @return this quantity after subtracting y
             **/
            template <typename Quantity2>
            quantity & operator-=(Quantity2 y) {
                static_assert(same_dimension_v<unit_type, typename Quantity2::unit_type>);

                quantity y2 = y;

                /* relying on assignment that correctly converts-to-lhs-units */
                this->scale_ -= y2.scale();

                return *this;
            }
            ///@}

            /** @defgroup quantity-comparisonsupport **/
            ///@{
            /** @brief compare this quantity with another, return 3-way comparison
             *
             *  @pre arguments must be quantities having the same dimension
             *
             *  @param y rhs quantity to compare
             *  @return signed integer;  {-ve, 0, +ve} when @c *this is {less than, equal, greater than} @p y
             **/
            template <typename Quantity2>
            requires quantity_concept<Quantity2> && same_dimension_v<unit_type, typename Quantity2::unit_type>
            auto compare(Quantity2 y) const {
                /* convert y to same {units, repr} as *this */
                quantity y2 = y;

                auto cmp = (this->scale_ <=> y2.scale());

                return cmp;
            }
            ///@}

            /** @defgroup quantity-comparison **/
            ///@{
            /** @brief 3-way comparison of two quantities
             *
             *  @pre arguments must be quantities having the same dimension
             *
             *  @param y rhs quantity to compare
             *  @return std::partial_ordering
             **/
            template <typename Quantity2>
            requires quantity_concept<Quantity2> && same_dimension_v<unit_type, typename Quantity2::unit_type>
            auto operator<=>(Quantity2 y) const {
                return this->compare(y);
            }

            /** @brief compare two quantities for equality
             *
             *  Although compiler generates this (due to presence of 3-way comparison operator),
             *  it flags ambiguous overload when included alongside .h files from std::distribution.
             *  Look like ambiguity would need to be resolved by a header change
             **/
            template <typename Quantity2>
            requires quantity_concept<Quantity2> && same_dimension_v<unit_type, typename Quantity2::unit_type>
            bool operator==(Quantity2 y) const {
                return std::is_eq(this->compare(y));
            }

            template <typename Quantity2>
            requires quantity_concept<Quantity2> && same_dimension_v<unit_type, typename Quantity2::unit_type>
            bool operator!=(Quantity2 y) const {
                return std::is_neq(this->compare(y));
            }


            /** @addtogroup mpl-quantity-unit-conversion **/
            ///@{
            /** @brief convert to quantity with same dimension, different {unit_type, repr_type}
             *
             *  @pre @c Quantity2 must have the same dimension as @c *this.
             **/
            template <typename Quantity2>
            requires quantity_concept<Quantity2> && same_dimension_v<unit_type, typename Quantity2::unit_type>
            constexpr operator Quantity2 () const {
                /* avoid truncating precision when converting:
                 * use best available representation
                 */
                using tmp_repr_type = std::common_type_t<repr_type, typename Quantity2::repr_type>;

                return Quantity2::promote(this->in_units_of<typename Quantity2::unit_type, tmp_repr_type>());
            }
            ///@}

            /** @defgroup mpl-quantity-print-support **/
            ///@{
            /** @brief write printed representation on stream
             *
             *  @param os   write on this output stream
             **/
            void display(std::ostream & os) const {
                os << this->scale() << unit_cstr();
            }
            ///@}

            /** @defgroup mpl-quantity-assignment **/
            ///@{
            /** @brief copy constructor **/
            quantity & operator=(quantity const & x) = default;
            /** @brief move constructor **/
            quantity & operator=(quantity && x) = default;
            ///@}

        private:
            explicit constexpr quantity(Repr x) : scale_{x} {}

            friend class promoter<Unit, Repr, true>;
            friend class promoter<Unit, Repr, false>;

        private:
            /** @brief quantity represents this multiple of a unit (that has compile-time outer-scalefactor of 1) **/
            Repr scale_ = 0;
        }; /*quantity*/

        template <typename Unit, typename Repr>
        constexpr auto
        quantity<Unit, Repr>::promote(Repr x) {
            //std::cerr << "quantity<U,R>::promote: x=" << x << ", R=" << reflect::Reflect::require<Repr>()->canonical_name() << std::endl;
            return promoter<Unit, Repr>::promote(x);
        }

        // ----- operator+ -----

        template <typename Quantity1, typename Quantity2>
        inline constexpr Quantity1 operator+ (Quantity1 x, Quantity2 y) {
            static_assert(same_dimension_v<typename Quantity1::unit_type, typename Quantity2::unit_type>);

            /* convert y to match units used by x;
             * would fail at compile time if this isn't well-defined
             */
            Quantity1 y2 = y;

            return Quantity1::promote(x.scale() + y2.scale());
        }

        template <typename Quantity1, typename Quantity2>
        inline constexpr Quantity1 operator- (Quantity1 x, Quantity2 y) {
            static_assert(std::same_as
                          < typename Quantity1::unit_type::dimension_type::canon_type,
                          typename Quantity2::unit_type::dimension_type::canon_type >);

            /* convert y to match units used by x */
            Quantity1 y2 = y;

            return Quantity1::promote(x.scale() - y2.scale());
        }

        template <typename Quantity>
        inline Quantity operator- (Quantity x) {
            return Quantity::promote(- x.scale());
        }

        template <typename Quantity1, typename Quantity2>
        inline constexpr auto operator* (Quantity1 x, Quantity2 y) {
            static_assert(quantity_concept<Quantity1>);
            static_assert(quantity_concept<Quantity2>);

            return x.multiply(y);
        }

        /** e.g. DECLARE_LH_MULT(int32_t) **/
#      define DECLARE_LH_MULT(lhtype)                  \
        template <typename Quantity>                   \
        inline constexpr auto                          \
        operator* (lhtype x, Quantity y) {             \
            static_assert(quantity_concept<Quantity>); \
            return y.scale_by(x);                      \
        }

        DECLARE_LH_MULT(int8_t);
        DECLARE_LH_MULT(uint8_t);
        DECLARE_LH_MULT(int16_t);
        DECLARE_LH_MULT(uint16_t);
        DECLARE_LH_MULT(int32_t);
        DECLARE_LH_MULT(uint32_t);
        DECLARE_LH_MULT(int64_t);
        DECLARE_LH_MULT(uint64_t);
        DECLARE_LH_MULT(float);
        DECLARE_LH_MULT(double);
#      undef DECLARE_LH_MULT

        /** e.g. DECLARE_RH_MULT(int32_t) **/
#      define DECLARE_RH_MULT(rhtype)                  \
        template <typename Quantity>                   \
        inline constexpr auto                          \
        operator* (Quantity x, rhtype y) {             \
            static_assert(quantity_concept<Quantity>); \
            return x.scale_by(y);                      \
        }

        DECLARE_RH_MULT(int8_t);
        DECLARE_RH_MULT(uint8_t);
        DECLARE_RH_MULT(int16_t);
        DECLARE_RH_MULT(uint16_t);
        DECLARE_RH_MULT(int32_t);
        DECLARE_RH_MULT(uint32_t);
        DECLARE_RH_MULT(int64_t);
        DECLARE_RH_MULT(uint64_t);
        DECLARE_RH_MULT(float);
        DECLARE_RH_MULT(double);
#      undef DECLARE_LH_MULT

        template <typename Quantity1, typename Quantity2>
        inline constexpr auto operator/ (Quantity1 x, Quantity2 y) {
            static_assert(quantity_concept<Quantity1>);
            static_assert(quantity_concept<Quantity2>);

            return x.divide(y);
        }

#      define DECLARE_LH_DIV(lhtype)                   \
        template <typename Quantity>                   \
        inline constexpr auto                          \
        operator/ (lhtype x, Quantity y) {             \
            static_assert(quantity_concept<Quantity>); \
            return y.divide_into(x);                   \
        }

        DECLARE_LH_DIV(int8_t);
        DECLARE_LH_DIV(uint8_t);
        DECLARE_LH_DIV(int16_t);
        DECLARE_LH_DIV(uint16_t);
        DECLARE_LH_DIV(int32_t);
        DECLARE_LH_DIV(uint32_t);
        DECLARE_LH_DIV(int64_t);
        DECLARE_LH_DIV(uint64_t);
        DECLARE_LH_DIV(float);
        DECLARE_LH_DIV(double);
#      undef DECLARE_LH_DIV

#      define DECLARE_RH_DIV(rhtype) \
        template <typename Quantity> \
        inline constexpr auto \
        operator/ (Quantity x, rhtype y) { \
            static_assert(quantity_concept<Quantity>); \
            return x.divide_by(y); \
        }

        DECLARE_RH_DIV(int8_t)
        DECLARE_RH_DIV(uint8_t)
        DECLARE_RH_DIV(int16_t)
        DECLARE_RH_DIV(uint16_t)
        DECLARE_RH_DIV(int32_t)
        DECLARE_RH_DIV(uint32_t)
        DECLARE_RH_DIV(int64_t)
        DECLARE_RH_DIV(uint64_t)
        DECLARE_RH_DIV(float)
        DECLARE_RH_DIV(double)
#      undef DECLARE_RH_DIV

        template <typename Unit, typename Repr>
        inline std::ostream &
        operator<< (std::ostream & os, quantity<Unit, Repr> const & x) {
            x.display(os);
            return os;
        }

        namespace qty {
            // ----- mass -----

            /** @brief create quantity representing @p x milligrams **/
            template <typename Repr = double>
            inline constexpr auto milligrams(Repr x) -> quantity<units::milligram, Repr> {
                return quantity<units::milligram, Repr>::promote(x);
            };

            /** @brief create quantity representing @p x grams **/
            template <typename Repr = double>
            inline constexpr auto grams(Repr x) -> quantity<units::gram, Repr> {
                return quantity<units::gram, Repr>::promote(x);
            };

            /** @brief create quantity representing @p x kilograms **/
            template <typename Repr = double>
            inline constexpr auto kilograms(Repr x) -> quantity<units::kilogram, Repr> {
                return quantity<units::kilogram, Repr>::promote(x);
            };

            // ----- distance -----

            /** @brief create quantity representing @p x millimeters **/
            template <typename Repr = double>
            inline constexpr auto millimeters(Repr x) -> quantity<units::millimeter, Repr> {
                return quantity<units::millimeter, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x meters **/
            template <typename Repr = double>
            inline constexpr auto meters(Repr x) -> quantity<units::meter, Repr> {
                return quantity<units::meter, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x kilometers **/
            template <typename Repr = double>
            inline constexpr auto kilometers(Repr x) -> quantity<units::kilometer, Repr> {
                return quantity<units::kilometer, Repr>::promote(x);
            }

            // ----- time -----

            /** @brief create quantity representing @p x nanoseconds **/
            template <typename Repr = double>
            inline constexpr auto nanoseconds(Repr x) -> quantity<units::nanosecond, Repr> {
                return quantity<units::nanosecond, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x microseconds **/
            template <typename Repr = double>
            inline constexpr auto microseconds(Repr x) -> quantity<units::microsecond, Repr> {
                return quantity<units::microsecond, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x milliseconds **/
            template <typename Repr = double>
            inline constexpr auto milliseconds(Repr x) -> quantity<units::millisecond, Repr> {
                return quantity<units::millisecond, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x seconds **/
            template <typename Repr = double>
            inline constexpr auto seconds(Repr x) -> quantity<units::second, Repr> {
                return quantity<units::second, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x minutes **/
            template <typename Repr = double>
            inline constexpr auto minutes(Repr x) -> quantity<units::minute, Repr> {
                return quantity<units::minute, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x hours **/
            template <typename Repr = double>
            inline constexpr auto hours(Repr x) -> quantity<units::hour, Repr> {
                return quantity<units::hour, Repr>::promote(x);
            }

            /** @brief create quantity representing @p x days (1 day = exactly 24 hours) **/
            template <typename Repr = double>
            inline constexpr auto days(Repr x) -> quantity<units::day, Repr> {
                return quantity<units::day, Repr>::promote(x);
            }

            // ----- time/volatility -----

            /** quantity in units of 1/sqrt(1dy) **/
            template <typename Repr = double>
            inline constexpr auto volatility1d(Repr x) -> quantity<units::volatility_1d, Repr> {
                return quantity<units::volatility_1d, Repr>::promote(x);
            }

            /** quantity in units of 1/sqrt(30days)
             **/
            template <typename Repr = double>
            inline constexpr auto volatility30d(Repr x) -> quantity<units::volatility_30d, Repr> {
                return quantity<units::volatility_30d, Repr>::promote(x);
            }

            /** quantity in units of 1/sqrt(250days)
             **/
            template <typename Repr = double>
            inline constexpr auto volatility250d(Repr x) -> quantity<units::volatility_250d, Repr> {
                return quantity<units::volatility_250d, Repr>::promote(x);
            }
        } /*namespace qty*/

        namespace unit_qty {
            /** @brief quantity with mass dimension, representing 1mg (1 milligram = 10^-3 grams) **/
            static constexpr auto milligram = qty::milligrams(1.0);
            /** @brief quantity with mass dimension, representing 1g (1 gram) **/
            static constexpr auto gram = qty::grams(1.0);
            /** @brief quantity with mass dimension, representing 1kg (1 kilogram = 1000 grams) **/
            static constexpr auto kilogram = qty::kilograms(1.0);

            /** @brief quantity with length dimension representing 1mm (10^-3 meters) **/
            static constexpr auto millimeter = qty::millimeters(1.0);
            /** @brief quantity with length dimension representing 1m (1 meter) **/
            static constexpr auto meter = qty::meters(1.0);
            /** @brief quantity with length dimension representing 1km (1 kilometer = 1000 meters) **/
            static constexpr auto kilometer = qty::kilometers(1.0);

            /** @brief quantity with time dimension representing 1ns (1 nanosecond = 10^-9 seconds) **/
            static constexpr auto nanosecond = qty::microseconds(1);
            /** @brief quantity with time dimension representing 1us (1 microsecond = 10^-6 seconds) **/
            static constexpr auto microsecond = qty::microseconds(1);
            /** @brief quantity with time dimension representing 1ms (1 milliseconds = 10^-3 seconds) **/
            static constexpr auto millisecond = qty::milliseconds(1);
            /** @brief quantity with time dimension representing 1s (1 second) **/
            static constexpr auto second = qty::seconds(1);
            /** @brief quantity with time dimension representing 1min (1 minute = 60 seconds) **/
            static constexpr auto minute = qty::minutes(1);
            /** @brief quantity with time dimension representing 1hr (1 hour = 60 minutes) **/
            static constexpr auto hour = qty::hours(1);
            /** @brief quantity with time dimension representing 1dy (1 day = 24 hours) **/
            static constexpr auto day = qty::days(1);
        }

    } /*namespace unit*/
} /*namespace xo*/

/* end quantity.hpp */
