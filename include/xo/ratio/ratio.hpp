/** @file ratio.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "ratio_concept.hpp"
#include "xo/flatstring/flatstring.hpp"
#include <numeric>
#include <compare>
//#include <type_traits>

namespace xo {
    namespace ratio {
        namespace detail {
            /** @brief converts ratio to lowest terms when feasible
             *
             * Falls back to identity function for non-totally-ordered Ratio::component_type
             */
            template <typename Ratio, bool EnabledFlag = std::totally_ordered<typename Ratio::component_type>>
            struct reducer_type;

            /** @brief promote value to ratio type **/
            template <typename Ratio, typename FromType, bool FromRatioFlag = ratio_concept<FromType>>
            struct promoter_type;

            /** @brief convert value to different numeric (or ratio) representation **/
            template <typename Ratio, typename ToType, bool ToRatioFlag = ratio_concept<ToType>>
            struct converter_type;
        }

        /** @class ratio
         *  @brief represent a ratio of two Int values.
         **/
        template <typename Int>
        requires std::totally_ordered<Int>
        struct ratio
        {
        public:
            /** @defgroup ratio-types **/
            ///@{
            /** @brief representation for (numerator, denominator) **/
            using component_type = Int;
            ///@}

        public:
            /** @defgroup ratio-ctor **/
            ///@{
            /** @brief construct ratio 0/1 **/
            constexpr ratio() = default;
            /** @brief construct ratio with numerator @p n and denominator @p d.
             *
             *  Ratio need not be normalized
             **/
            constexpr ratio(Int n, Int d) : num_{n}, den_{d} {}
            ///@}

            /** @defgroup ratio-static-methods **/
            ///@{
            /** @brief ratio in lowest commono terms
             *
             **/
            static constexpr ratio reduce(Int n, Int d) {
                return ratio(n, d).normalize();
            }

            /** @brief add ratios @p x and @p y
             *
             *  @post result ratio is normalized
             **/
            static constexpr ratio add(const ratio & x,
                                       const ratio & y) {
                /* (a/b) + (c/d)
                 *   = a.d / (b.d) + b.c / (b.d)
                 *   = (a.d + b.c) / (b.d)
                 */

                auto a = x.num();
                auto b = x.den();
                auto c = y.num();
                auto d = y.den();

                auto num = a*d + b*c;
                auto den = b*d;

                return ratio(num, den).maybe_reduce();
            }

            /** @brief subtract ratio @p y from ratio @p x
             *
             *  @post result ratio is normalized
             **/
            static constexpr ratio subtract(const ratio & x,
                                            const ratio & y) {
                return add(x, y.negate());
            }

            /** @brief multiply ratios @p x and @p y
             *
             *  @post result ratio is normalized
             **/
            static constexpr ratio multiply(const ratio & x,
                                            const ratio & y) {
                /* (a/b) * (c/d) = a.c / b.d */

                /* if x,y normalized,
                 * opportunity to cancel common factor between (a, d) or (c, b)
                 *
                 * want to do this before multiplying to avoid overflow involving intermediate terms
                 */

                auto a1 = x.num();
                auto b1 = x.den();
                auto c1 = y.num();
                auto d1 = y.den();

                auto ad_gcf = std::gcd(a1, d1);
                auto bc_gcf = std::gcd(b1, c1);

                auto a = a1 / ad_gcf;
                auto b = b1 / bc_gcf;
                auto c = c1 / bc_gcf;
                auto d = d1 / ad_gcf;

                auto num = a*c;
                auto den = b*d;

                return ratio(num, den).maybe_reduce();
            }

            /** @brief divide ratio @p y into ratio @p x
             *
             *  @post result ratio is normalized
             **/
            static constexpr ratio divide(const ratio & x,
                                          const ratio & y)
                {
                    return multiply(x, y.reciprocal());
                }

            /** @brief 3-way compare two ratios **/
            static constexpr auto compare(ratio x, ratio y) {
                /* ensure minus signs in numerators only */
                if (x.den() < 0)
                    return compare_aux(ratio(-x.num(), -x.den()), y);
                if (y.den() < 0)
                    return compare_aux(x, ratio(-y.num(), -y.den()));

                return compare_aux(x, y);
            }
            ///@}

            /** @defgroup ratio-access **/
            ///@{
            /** @brief fetch ratio's numerator **/
            constexpr Int num() const noexcept { return num_; }
            /** @brief fetch ratio's denominator **/
            constexpr Int den() const noexcept { return den_; }

            /** @brief true if and only if ratio is equal to zero **/
            constexpr bool is_zero() const noexcept { return (num_ == 0) && (den_ != 0); }

            /** @brief true if and only if ratio is equal to one **/
            constexpr bool is_unity() const noexcept { return (num_ != 0) && (num_ == den_); }

            /** @brief true if and only if ratio represents an integer
             *
             *  (denominator is +/- 1)
             **/
            constexpr bool is_integer() const noexcept { return den_ == 1 || den_ == -1; }
            ///@}

            /** @defgroup ratio-methods **/
            ///@{
            /** @brief r.negate() is the exact ratio representing @c -r **/
            constexpr ratio negate() const { return ratio(-num_, den_); }
            /** @brief r.reciprocal() is the eact ratio representing @c 1/r **/
            constexpr ratio reciprocal() const { return ratio(den_, num_); }

            /** @brief r.floor() is the largest integer x : x <= r **/
            constexpr Int floor() const { return (num_ / den_); }

            /** @brief r.ceil() is the smallest integer x : r < x. **/
            constexpr Int ceil() const { return floor() + 1; }

            /** @brief reduce to lowest terms
             *
             *  @pre @c Int type must be totally ordered
             **/
            constexpr ratio normalize() const requires std::totally_ordered<Int> {
                if (den_ < 0)
                    return ratio(-num_, -den_).normalize();

                auto factor = std::gcd(num_, den_);

                return ratio(num_ / factor,
                             den_ / factor);
            }

            /** @brief reduce to lowest terms,  if Int representation admits
             *
             *  Otherwise fallback to identity function
             **/
            constexpr ratio maybe_reduce() const {
                return detail::reducer_type<ratio>::attempt_reduce(*this);
            }

            /** @brief return fractional part of this ratio
             *
             *  @pre @c Int type must be totally ordered
             **/
            constexpr ratio frac() const requires std::totally_ordered<Int> {
                return ratio::subtract(*this, ratio(this->floor(), 1));
            }

            /** @brief compute integer exponent @p p of this ratio
             *
             *  @note time complexity is O(log p)
             **/
            constexpr ratio power(int p) const {

                constexpr ratio retval = ratio(1, 1);

                if (p == 0)
                    return ratio(1, 1);

                if (p < 0)
                    return this->reciprocal().power(-p);

                /* inv: x^p = aj.xj^pj */
                ratio aj = ratio(1, 1);
                ratio xj = *this;
                int pj = p;

                while (pj > 0) {
                    if (pj % 2 == 0) {
                        /* a.x^(2q) = a.(x^2)^q */
                        xj = xj * xj;
                        pj = pj / 2;
                    } else {
                        /* a.x^(2q+1) = (a.x).x^(2q) */
                        aj = aj * xj;
                        pj = (pj - 1);
                    }
                }

                /* pj = 0, so: x^p = aj.xj^pj = aj.xj^0 = aj */
                return aj;
            }

            /** @brief negate operator **/
            constexpr ratio operator-() const { return ratio(-num_, den_); }
            ///@}

            /** @defgroup ratio-conversion **/
            ///@{
            /** @brief convert to non-ratio representation
             *
             *  For example: to int or double
             **/
            template <typename Repr>
            constexpr Repr convert_to() const noexcept {
                return detail::converter_type<ratio, Repr>::convert(*this);
            }

            /** @brief convert to short human-friendly flatstring representation
             *
             *  Example:
             *  @code
             *  ratio(7,1).to_str<5>();   // "7"
             *  ratio(1,7).to_str<5>();   // "(1/7)"
             *  ratio(-1,7).to_str<10>(); // "(-1/7)"
             *  ratio(-1,7).to_str<5>();  // "(-1/"
             *  @endcode
             **/
            template <std::size_t N>
            constexpr flatstring<N> to_str() const noexcept {
                if (this->is_integer()) {
                    return flatstring<N>::from_int(num_);
                } else {
                    auto num_str = flatstring<N>::from_int(num_);
                    auto den_str = flatstring<N>::from_int(den_);

                    /* tmp capacity will be about 2N+3 */
                    auto tmp = flatstring_concat(flatstring("("),
                                                 num_str,
                                                 flatstring("/"),
                                                 den_str,
                                                 flatstring(")"));

                    flatstring<N> retval;
                    retval.assign(tmp);

                    return retval;
                }
            }

            /** @brief convert to representation using different integer types **/
            template <typename Ratio2>
            constexpr operator Ratio2 () const noexcept requires ratio_concept<Ratio2> {
                return Ratio2(num_, den_);
            }
            ///@}

        private:
            /** @brief 3-way compare auxiliary function.
             *
             *  @pre @p x, @p y have non-negative denominator
             **/
            static constexpr auto compare_aux(ratio x, ratio y) noexcept {
                /* control here: b>=0, d>=0 */

                /* (a/b)   <=> (c/d)
                 * (a.d/b) <=> c       no sign change, since d >= 0
                 * (a.d)   <=> (b.c)   no sign change, since b >= 0
                 */
                auto a = x.num();
                auto b = x.den();
                auto c = y.num();
                auto d = y.den();

                auto lhs = a*d;
                auto rhs = b*c;

                return lhs <=> rhs;
            }

            /* we want ratio<T> to be a structural type,
             * so that we can use an instance as a non-type template parameter.
             *
             * with private members, clang 18 (we believe incorrectly) complains that ratio<T> is not structural.
             */
#ifdef __clang__
        public:
#else
        private:
#endif

            /** @defgroup ratio-instance-variables **/
            ///@{
            /** @brief numerator **/
            Int num_ = 0;
            /** @brief denominator **/
            Int den_ = 1;
            ///@}
        };

        namespace detail {
            template <typename Ratio, bool EnabledFlag>
            struct reducer_type {};

            template <typename Ratio>
            struct reducer_type<Ratio, true /*EnabledFlag*/> {
                static constexpr Ratio attempt_reduce(Ratio x) { return x.normalize(); }
            };

            template <typename Ratio>
            struct reducer_type<Ratio, false /*!EnabledFlag*/> {
                static constexpr Ratio attempt_reduce(Ratio x) { return x; }
            };
        }

        namespace detail {
            template <typename Ratio, typename FromType, bool FromRatioFlag>
            struct promoter_type;

            template <typename Ratio, typename FromType>
            struct promoter_type<Ratio, FromType, true /*FromRatioFlag*/> {
                /* to 'promote' a ratio,  rely on its conversion operator */
                static constexpr Ratio promote(FromType x) { return x; }
            };

            template <typename Ratio, typename FromType>
            struct promoter_type<Ratio, FromType, false /*!FromRatioFlag*/> {
                /* to 'promote' a non-ratio,  use denominator=1 */
                static constexpr Ratio promote(FromType x) { return Ratio(x, 1); }
            };
        }

        namespace detail {
            template <typename Ratio, typename ToType, bool ToRatioFlag>
            struct converter_type;

            template <typename Ratio, typename ToType>
            struct converter_type<Ratio, ToType, true /*ToRatioFlag*/> {
                /* to convert to a ratio,  can just use built-in conversion */
                static constexpr ToType convert(Ratio x) { return ToType(x.num(), x.den()); }
            };

            template <typename Ratio, typename ToType>
            struct converter_type<Ratio, ToType, false /*!ToRatioFlag*/> {
                /* to convert to non-ratio,  do division */
                static constexpr ToType convert(Ratio x) { return x.num() / static_cast<ToType>(x.den()); }
            };
        }

        /** @brief create a ratio in lowest terms from two integers **/
        template <typename Int1, typename Int2>
        constexpr auto
        make_ratio (Int1 n, Int2 d = 1) -> ratio<std::common_type_t<Int1, Int2>>
        {
            return ratio<std::common_type_t<Int1, Int2>>(n, d).maybe_reduce();
        }

        namespace detail {
            /** @brief auxiliary function for binary ratio operations
             *
             *  Support binary ratio operations on combinations:
             *  - (ratio<T>, ratio<U>)
             *  - (ratio<T>, U)    // where U is not a ratio
             *  - (T, ratio(U))    // where T is not a ratio
             *
             *  Goals:
             *
             *  1. Support expressions like
             *
             *  @code
             *     auto x = 1 + make_ratio(2,3);
             *  @endcode
             *
             *  2. promote to wider types as needed
             *
             *  @code
             *     auto x = make_ratio(2,3) + make_ratio(1ul,2ul);
             *     static_assert(std::same_as<x::component_type, unsigned long>);
             *  @endcode
             *
             *  3. avoid interfering with other templates that may overload operator+
             *
             *  @pre at least one of (Left,Right) must be known to be a ratio
             **/
            template <typename Left,
                      typename Right,
                      bool LeftIsRatio = ratio_concept<Left>,
                      bool RightIsRatio = ratio_concept<Right>>
            struct op_aux_type;

            /** @brief specialization for two ratio types **/
            template <typename LeftRatio,
                      typename RightRatio>
            requires (ratio_concept<LeftRatio> && ratio_concept<RightRatio>)
            struct op_aux_type<LeftRatio, RightRatio, true /*LeftIsRatio*/, true /*RightIsRatio*/> {
                using component_type = std::common_type_t<typename LeftRatio::component_type,
                                                          typename RightRatio::component_type>;

                using ratio_type = ratio<component_type>;

                static constexpr ratio_type add (const LeftRatio & x,
                                                 const RightRatio & y)
                    {
                        return ratio_type::add(x, y);
                    }

                static constexpr ratio_type subtract (const LeftRatio & x,
                                                      const RightRatio & y)
                    {
                        return ratio_type::subtract(x, y);
                    }

                static constexpr ratio_type multiply (const LeftRatio & x,
                                                      const RightRatio & y)
                    {
                        return ratio_type::multiply(x, y);
                    }

                static constexpr ratio_type divide (const LeftRatio & x,
                                                      const RightRatio & y)
                    {
                        return ratio_type::divide(x, y);
                    }

                static constexpr auto compare (const LeftRatio & x,
                                               const RightRatio & y)
                    {
                        return ratio_type::compare(x, y);
                    }
            };

            /** @brief specialization for left-hand ratio and right-hand integer value **/
            template <typename LeftRatio,
                      typename Right>
            requires (ratio_concept<LeftRatio> && !ratio_concept<Right>)
            struct op_aux_type<LeftRatio, Right, true /*LeftIsRatio*/, false /*RightIsRatio*/> {
                using component_type = std::common_type_t<typename LeftRatio::component_type, Right>;

                using ratio_type = ratio<component_type>;

                static constexpr ratio_type add (const LeftRatio & x,
                                                 const Right & y)
                    {
                        /* reminder: adding an integer can't introduce reduced terms */
                        return ratio_type(x.num() + x.den() * y, x.den());
                    }

                static constexpr ratio_type subtract (const LeftRatio & x,
                                                      const Right & y)
                    {
                        /* reminder: subtracting an integer can't introduce reduced terms */
                        return ratio_type(x.num() - x.den() * y, x.den());
                    }

                static constexpr ratio_type multiply (const LeftRatio & x,
                                                      const Right & yp)
                    {
                        auto gcf = std::gcd(x.den(), yp);

                        auto a = x.num();
                        auto b = x.den() / gcf;
                        auto y = yp / gcf;

                        return ratio_type(a*y, b);
                    }

                static constexpr ratio_type divide (const LeftRatio & x,
                                                    const Right & yp)
                    {
                        auto gcf = std::gcd(x.num(), yp);

                        auto a = x.num() / gcf;
                        auto b = x.den();
                        auto y = yp / gcf;

                        return ratio_type(a*y, b);
                    }

                static constexpr auto compare (const LeftRatio & x,
                                               const Right & y)
                    {
                        /* note: in c++26 std::signof is constexpr,  usable here */
                        if (x.den() >= 0)
                            return compare_aux(x, y);
                        else
                            return compare_aux(LeftRatio(-x.num(), -x.den()), y);
                    }

            private:
                static constexpr auto compare_aux (const LeftRatio & x, const Right & y) {
                    return (x.num() <=> x.den() * y);
                }
            };

            /** @brief specialization for left-hand integer value and right-hand ratio **/
            template <typename Left,
                      typename RightRatio>
            requires (!ratio_concept<Left> && ratio_concept<RightRatio>)
            struct op_aux_type<Left, RightRatio, false /*LeftIsRatio*/, true /*RightIsRatio*/> {
                using component_type = std::common_type_t<Left, typename RightRatio::component_type>;

                using ratio_type = ratio<component_type>;

                static constexpr ratio_type add(const Left & x,
                                                const RightRatio & y)
                    {
                        /* reminder: adding an integer can't introduce reduced terms */
                        return ratio_type(x * y.den() + y.num(), y.den());
                    }

                static constexpr ratio_type subtract(const Left & x,
                                                     const RightRatio & y)
                    {
                        /* reminder: subtracting an integer can't introduce reduced terms */
                        return ratio_type(x * y.den() - y.num(), y.den());
                    }

                static constexpr ratio_type multiply (const Left & xp,
                                                      const RightRatio & y)
                    {
                        auto gcf = std::gcd(xp, y.den());

                        auto x = xp / gcf;
                        auto c = y.num();
                        auto d = y.den() / gcf;

                        return ratio_type(x*c, d);
                    }

                static constexpr ratio_type divide (const Left & x,
                                                    const RightRatio & y)
                    {
                        return multiply(x, y.reciprocal());
                    }

                static constexpr auto compare(const Left & x,
                                              const RightRatio & y)
                    {
                        if (y.den() >= 0)
                            return compare_aux(x, y);
                        else
                            return compare_aux(x, RightRatio(-y.num(), -y.den()));
                    }

            private:
                static constexpr auto compare_aux (const Left & x,
                                                   const RightRatio & y)
                    {
                        return (x * y.den() <=> y.num());
                    };
            };
        } /*namespace detail*/

        /** @defgroup ratio-arithmetic **/
        ///@{
        /** @brief add two ratios.
         *
         *  One argument may be a non-ratio type if it can be promoted to a ratio
         **/
        template <typename Ratio1, typename Ratio2>
        inline constexpr auto
        operator+ (const Ratio1 & x, const Ratio2 & y)
            requires (ratio_concept<Ratio1> || ratio_concept<Ratio2>)
        {
            return detail::op_aux_type<Ratio1, Ratio2>::add(x, y);
        }

        /** @brief subtract two ratios.
         *
         *  One argument may be a non-ratio type if it can be promoted to a ratio
         **/
        template <typename Ratio1, typename Ratio2>
        inline constexpr auto
        operator- (const Ratio1 & x, const Ratio2 & y)
            requires (ratio_concept<Ratio1> || ratio_concept<Ratio2>)
        {
            return detail::op_aux_type<Ratio1, Ratio2>::subtract(x, y);
        }

        /** @brief multiply two ratios
         *
         *  One argument may be a non-ratio type if it can be promoted to a ratio
         **/
        template <typename Ratio1, typename Ratio2>
        inline constexpr auto
        operator* (const Ratio1 & x, const Ratio2 & y)
            requires (ratio_concept<Ratio1> || ratio_concept<Ratio2>)
        {
            return detail::op_aux_type<Ratio1, Ratio2>::multiply(x, y);
        }

        /** @brief divide two ratios
         *
         *  One argument may be a non-ratio type if it can be promoted to a ratio
         **/
        template <typename Ratio1, typename Ratio2>
        inline constexpr auto
        operator/ (const Ratio1 & x, const Ratio2 & y)
            requires (ratio_concept<Ratio1> || ratio_concept<Ratio2>)
        {
            return detail::op_aux_type<Ratio1, Ratio2>::divide(x, y);
        }
        ///@}

        /** @defgroup ratio-3way-compare 3way comparison **/
        ///@{
        /** @brief compare two ratios for equality
         *
         *  One argument may be a non-ratio type if it can be promoted to a ratio
         **/
        template <typename Ratio1, typename Ratio2>
        inline constexpr bool
        operator== (const Ratio1 & x, const Ratio2 & y)
            requires (ratio_concept<Ratio1> || ratio_concept<Ratio2>)
        {
            return (detail::op_aux_type<Ratio1, Ratio2>::compare(x, y) == 0);
        }

        /** @brief compare two ratios
         *
         *  One argument may be a non-ratio type if it can be promoted to a ratio
         **/
        template <typename Ratio1, typename Ratio2>
        inline constexpr auto
        operator<=> (const Ratio1 & x, const Ratio2 & y)
            requires (ratio_concept<Ratio1> || ratio_concept<Ratio2>)
        {
            return detail::op_aux_type<Ratio1, Ratio2>::compare(x, y);
        }
        ///@}

    } /*namespace ratio*/
} /*namespace xo*/

/** end ratio.hpp **/
