/* @file normal_dist.hpp */

#pragma once

#include <limits>
#include <cmath>
#include <cstdint>

namespace xo {
    namespace rng {
        /* normally-distributed r.v's, by the Marsaglia polar method.
         *
         * Interface-compatible with std::normal_distribution<FloatType>,
         * and used in its place because the standard specifies exact output
         * for engines but NOT for distributions: libstdc++ and libc++ both
         * generate two variates at a time and cache one, but return the pair
         * in opposite order.  Sequences therefore differ by an adjacent-pair
         * swap, which makes any pinned output (see
         * xo-kalmanfilter/utest/utestdata/) valid on one standard library and
         * not the other.
         *
         * Owning the transform makes the sequence a property of xo rather
         * than of whichever library is underneath.
         */
        template <typename FloatType>
        class normal_dist {
        public:
            using result_type = FloatType;

            class param_type {
            public:
                using distribution_type = normal_dist<FloatType>;

                param_type() = default;
                param_type(FloatType mean, FloatType sdev)
                    : mean_{mean}, sdev_{sdev} {}

                FloatType mean() const { return mean_; }
                FloatType stddev() const { return sdev_; }

                bool operator==(const param_type & x) const = default;

            private:
                FloatType mean_ = 0.0;
                FloatType sdev_ = 1.0;
            }; /*param_type*/

        public:
            normal_dist() = default;
            explicit normal_dist(FloatType mean, FloatType sdev = 1.0)
                : param_{mean, sdev} {}
            explicit normal_dist(const param_type & p) : param_{p} {}

            FloatType mean() const { return param_.mean(); }
            FloatType stddev() const { return param_.stddev(); }

            /* discard the cached second variate of the current pair */
            void reset() { cached_flag_ = false; }

            param_type param() const { return param_; }
            void param(const param_type & p) { param_ = p; }

            result_type min() const {
                return -std::numeric_limits<FloatType>::infinity();
            }
            result_type max() const {
                return +std::numeric_limits<FloatType>::infinity();
            }

            template <typename Engine>
            result_type operator()(Engine & eng) {
                return (*this)(eng, this->param_);
            }

            template <typename Engine>
            result_type operator()(Engine & eng, const param_type & p) {
                return p.mean() + p.stddev() * this->next_n01(eng);
            }

            bool operator==(const normal_dist & x) const {
                return ((param_ == x.param_)
                        && (cached_flag_ == x.cached_flag_)
                        && (!cached_flag_ || (cached_ == x.cached_)));
            }
            bool operator!=(const normal_dist & x) const { return !(*this == x); }

        private:
            /* uniform on the open interval (0,1).
             *
             * NB computed here rather than with std::uniform_real_distribution
             * or std::generate_canonical -- both are also implementation-defined,
             * so either would reintroduce the divergence this class exists to
             * remove.  Open interval because the polar method below takes log(s).
             */
            template <typename Engine>
            static FloatType uniform01(Engine & eng) {
                constexpr std::uint64_t c_range
                    = (static_cast<std::uint64_t>(Engine::max())
                       - static_cast<std::uint64_t>(Engine::min()));

                std::uint64_t u = (static_cast<std::uint64_t>(eng())
                                   - static_cast<std::uint64_t>(Engine::min()));

                return ((static_cast<FloatType>(u) + 0.5)
                        / (static_cast<FloatType>(c_range) + 1.0));
            }

            /* Marsaglia polar method: rejection-sample a point in the unit
             * disc, from which two independent N(0,1) variates fall out.
             * Return the first; cache the second for the next call.
             */
            template <typename Engine>
            FloatType next_n01(Engine & eng) {
                if (cached_flag_) {
                    cached_flag_ = false;
                    return cached_;
                }

                FloatType u, v, s;

                do {
                    u = 2.0 * uniform01(eng) - 1.0;
                    v = 2.0 * uniform01(eng) - 1.0;
                    s = u*u + v*v;
                } while ((s >= 1.0) || (s == 0.0));

                FloatType f = std::sqrt(-2.0 * std::log(s) / s);

                cached_ = v * f;
                cached_flag_ = true;

                return u * f;
            }

        private:
            param_type param_;
            /* true iff cached_ holds the unused second variate of a pair */
            bool cached_flag_ = false;
            FloatType cached_ = 0.0;
        }; /*normal_dist*/
    } /*namespace rng*/
} /*namespace xo*/

/* end normal_dist.hpp */
