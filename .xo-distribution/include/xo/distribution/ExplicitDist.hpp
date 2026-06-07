/* file ExplicitDist.hpp
 *
 * author: Roland Conybeare, Oct 2022
 */

#pragma once

#include "Distribution.hpp"
#include "Normal.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/tostr.hpp"
#include <vector>
#include <cmath>
#include <cstdint>

namespace xo {
    using xo::xtag;

    namespace distribution {
        class ProbabilityBucket {
        public:
            ProbabilityBucket() = default;

            double weight() const { return wt_; }
            double cdf() const { return cdf_; }

            /* note: when calling this, must invalidate ExplicitDist.cdf_valid_flag */
            void scale_weight(double k) { this->wt_ *= k; }

            void assign_weight(double w) { this->wt_ = w; }
            /* implementation method: only ExplicitDist.renormalize() should call this */
            void assign_cdf(double x) { this->cdf_ = x; }

            void display(std::ostream & os) const {
                using xo::xtag;

                os << "<ProbabilityBucket"
                   << xtag("wt", this->wt_)
                   << xtag("cdf", this->cdf_)
                   << ">";
            } /*display*/

        private:
            /* probability assigned to this bucket.
             * updated in-place by ExplicitDist.include_sample()
             */
            double wt_ = 0.0;
            /* cumulative probability assigned to this bucket b
             * and all buckets b[i] for domain values < b
             *
             * invalidated by ExplicitDist.scale_bucket()
             * see ExplicitDist.
             */
            double cdf_ = 0.0;
        }; /*ProbabilityBucket*/

        inline std::ostream & operator<<(std::ostream & os, ProbabilityBucket const & x) {
            x.display(os);
            return os;
        } /*operator<<*/

        namespace detail {
            /* three kinds of index here:
             * 1. lo:     non-negative index into ExplicitDist.lo_v_[]
             * 2. hi:     non-negative index into ExplicitDist.hi_v_[]
             * 3. signed: if >=0: index into ExplicitDist.hi_v_[]
             *            if  <0: index into ExplicitDist.lo_v_[]
             *
             * with lz=.lo_v.size(), hz=hi_v.size():
             *
             *                            .hi_v[0]     .hi_v[hz-1]
             *                          | v            v
             *                          |
             *        +--+--+     +--+--|--+--+      +--+
             *        |  |  | ... |  |  |  |  |  ... |  |
             *        +--+--+     +--+--|--+--+      +--+
             *          ^              ^
             *          .lo_v[lz-1]    .lo_v[0]
             *
             *          ^              ^  ^            ^
             * signed -lz             -1| 0      ...   hz-1
             */
            class ExplicitDistIndexUtil {
            public:
                static size_t signed2hi(int32_t signed_ix) { return signed_ix; }
                static size_t signed2lo(int32_t signed_ix) { return -signed_ix - 1; }
                static int32_t lo2signed(size_t lo_ix) { return -static_cast<int32_t>(lo_ix)-1; }
                static int32_t hi2signed(size_t hi_ix) { return hi_ix; }

                static ProbabilityBucket const * signed2bucket_c(int32_t signed_ix,
                                                                 std::vector<ProbabilityBucket> const * lo_v,
                                                                 std::vector<ProbabilityBucket> const * hi_v) {
                    if (signed_ix >= 0) {
                        size_t hi_ix = signed2hi(signed_ix);

                        if (hi_v && (hi_ix < hi_v->size()))
                            return &(*hi_v)[hi_ix];
                    } else {
                        size_t lo_ix = signed2lo(signed_ix);

                        if (lo_v && (lo_ix < lo_v->size()))
                            return &(*lo_v)[lo_ix];
                    }

                    return nullptr;
                } /*signed2bucket_c*/

                static ProbabilityBucket * signed2bucket(int32_t signed_ix,
                                                         std::vector<ProbabilityBucket> * lo_v,
                                                         std::vector<ProbabilityBucket> * hi_v) {
                    ProbabilityBucket const * b = signed2bucket_c(signed_ix, lo_v, hi_v);
                    return const_cast<ProbabilityBucket *>(b);
                } /*signed2bucket*/

            }; /*ExplicitDistIndexUtil*/

            class ExplicitDistIterator : public ExplicitDistIndexUtil {
            public:
                ExplicitDistIterator() = default;
                ExplicitDistIterator(int32_t signed_ix,
                                     std::vector<ProbabilityBucket> * lo_v,
                                     std::vector<ProbabilityBucket> * hi_v)
                    : signed_ix_{signed_ix}, p_lo_v_{lo_v}, p_hi_v_{hi_v} {}

                ProbabilityBucket & operator*() {
                    ProbabilityBucket * pb = signed2bucket(this->signed_ix_,
                                                           this->p_lo_v_,
                                                           this->p_hi_v_);
                    if (!pb) {
                        throw std::runtime_error("ExplicitDistIterator: attempt to deref invalid iterator");
                    }

                    return *pb;
                } /*operator**/

                ExplicitDistIterator & operator++() { ++(this->signed_ix_); return *this; }
                ExplicitDistIterator & operator--() { --(this->signed_ix_); return *this; }

            private:
                /* signed iterator */
                int32_t signed_ix_ = 0;
                /* will be ExplicitDist.lo_v[] */
                std::vector<ProbabilityBucket> * p_lo_v_ = nullptr;
                /* will be ExplicitDist.hi_v[] */
                std::vector<ProbabilityBucket> * p_hi_v_ = nullptr;
            }; /*ExplicitDistIterator*/
        } /*namespace detail*/

        /* explicit distribution with buckets.
         * (if you want to avoid bucketing at the expense of memory,
         *  see StdEmpirical)
         *
         * sample-independent buckets can be faster to the extent
         *    {#of buckets} << {#of samples}
         *
         * in particular
         *                                 | StdEmpirical | PackedEmpirical
         *   ------------------------------+--------------+-----------------
         *   update existing sample/bucket |   O(log(n))  |    O(1)
         *   create new sample/bucket      |   O(log(n))  |    O(log(n))
         *   cdf                           |   O(log(n))  |    O(n)
         *
         * PackedEmpirical offers scaling + renormalization
         *
         * since .cdf() is slow,  .ks_stat_1sided() is supported only in StdEmpirical.
         * (could generalize in future to some other implementation with fast .cdf())
         *
         * Require:
         *   Domain is something with metric,  e.g. int|double.
         *   categorical domain not supported here.
         *
         * see also: statistics/Histogram.  Histogram keeps more per-bucket statistics
         */
        template<typename Domain>
        class ExplicitDist : public Distribution<Domain>, public detail::ExplicitDistIndexUtil {
        public:
            using WeightVector = std::vector<ProbabilityBucket>;
            using iterator = detail::ExplicitDistIterator;

        public:
            static rp<ExplicitDist> make(Domain bucket_dx, Domain ref_value) {
                return new ExplicitDist(bucket_dx, ref_value);
            }
            /* create distribution with n buckets of width bucket_dx,
             * covering range [ref_value, ref_value + n * bucket_dx]
             */
            static rp<ExplicitDist> make_n(size_t n, Domain bucket_dx, Domain ref_value) {
                return new ExplicitDist(n, bucket_dx, ref_value);
            }

            size_t n_bucket() const { return this->lo_v_.size() + this->hi_v_.size(); }

            /* lub domain value with .cdf(lo) = 0 */
            Domain lo() const {
                std::size_t lz = this->lo_v_.size();

                return this->ref_value_ - lz * this->bucket_dx_;
            } /*lo*/

            /* glb domain value with .cdf(hi) = 1 */
            Domain hi() const {
                std::size_t hz = this->hi_v_.size();

                return this->ref_value_ + hz * this->bucket_dx_;
            } /*hi*/

            Domain bucket_lo(int32_t signed_ix) const {
                return this->ref_value_ + signed_ix * this->bucket_dx_;
            } /*bucket_lo*/

            Domain bucket_mid(int32_t signed_ix) const {
                return this->ref_value_ + (0.5 + signed_ix) * this->bucket_dx_;
            } /*bucket_mid*/

            Domain bucket_hi(int32_t signed_ix) const {
                return this->bucket_lo(signed_ix + 1);
            } /*bucket_hi*/

            iterator begin() { return iterator(lo2signed(this->lo_v_.size() - 1),
                                               &(this->lo_v_),
                                               &(this->hi_v_)); }
            iterator end() { return iterator(hi2signed(this->hi_v_.size()),
                                             &(this->lo_v_),
                                             &(this->hi_v_)); }

            /* probability density at domain value x */
            double density(Domain const & x) const {
                /* careful!  may need to renormalize */
                {
                    ExplicitDist<Domain> * self = const_cast<ExplicitDist<Domain> *>(this);

                    self->check_renormalize();
                }

                auto v = this->lookup_bucket(x);
                ProbabilityBucket const * b = v.first;

                if (b) {
                    /* probability density is constant within each bucket */
                    return b->weight() / this->bucket_dx_;
                } else {
                    return 0.0;
                }
            } /*density*/

            /* pair {bucket glb, density} for all buckets,  in increasing domain order */
            std::vector<std::pair<Domain, double>> density_v() const {
                /* careful! may need to renormalize */
                {
                    ExplicitDist<Domain> * self = const_cast<ExplicitDist<Domain> *>(this);

                    self->check_renormalize();
                }

                size_t n = this->n_bucket();
                std::vector<std::pair<Domain, double>> retval(n);

                double w2d = 1.0 / this->bucket_dx_;

                for(size_t i=0; i<n; ++i) {
                    Domain lo = this->bucket_lo(i);
                    ProbabilityBucket const * b = this->lookup_signed_bucket(i);

                    assert(b);

                    double density = b->weight() * w2d;

                    retval[i] = std::make_pair(lo, density);
                }

                return retval;
            } /*density_v*/

            /* return:
             *   .first   signed bucket index ix;
             *            refers to .lo_v[1-ix] if ix<0;
             *            refers to .hi_v[+ix] if ix>=0
             *   .second  fractional weight within bucket associated with x.
             *            not scaled by ProbabilityWeight.weight
             */
            std::pair<int32_t, double> signed_bucket_index(Domain const & x) const {
                double ix_f = ::floor((x - this->ref_value_) / this->bucket_dx_);
                int32_t ix = ix_f;

                /*
                 *           ^
                 *           |............
                 *           |    :      |
                 *           | wt :      | unit area
                 *           |    :      |
                 *           +-----------+
                 *   bucket_lo    ^    bucket_lo + .bucket_dx
                 *                x
                 */

                Domain bucket_lo = this->ref_value_ + (ix * this->bucket_dx_);
                double wt = (x - bucket_lo) / this->bucket_dx_;

                return std::make_pair(ix, wt);
            } /*signed_bucket_index*/

            ProbabilityBucket const * lookup_signed_bucket(int32_t signed_ix) const {
                return signed2bucket_c(signed_ix, &(this->lo_v_), &(this->hi_v_));
            } /*lookup_signed_bucket*/

            /* non-const version with write access */
            ProbabilityBucket * lookup_signed_bucket(int32_t signed_ix) {
                return signed2bucket(signed_ix, &(this->lo_v_), &(this->hi_v_));
            } /*lookup_signed_bucket*/

            /* return null pointer if bucket that would contain x not represented
             *   .second is fractional weight (relative to unity) within selected bucket
             */
            std::pair<ProbabilityBucket const *, double> lookup_bucket(Domain const & x) const {
                /* signed index.
                 *   ix>=0 => .lo_v[]
                 *   ix< 0 => .hi_v[]
                 */
                std::pair<int32_t, double> v = this->signed_bucket_index(x);

                int32_t ix = v.first;
                double fraction_wt = v.second;
                ProbabilityBucket const * pw = this->lookup_signed_bucket(ix);

                return std::make_pair(pw, fraction_wt);
            } /*lookup_bucket*/

            /* non-const version */
            std::pair<ProbabilityBucket *, double> lookup_bucket(Domain const & x) {
                ExplicitDist<Domain> const * c_self = this;

                std::pair<ProbabilityBucket const *, double> v = c_self->lookup_bucket(x);
                ProbabilityBucket * b = const_cast<ProbabilityBucket *>(v.first);
                double fraction_wt = v.second;

                return std::make_pair(b, fraction_wt);
            } /*lookup_bucket*/

            /* like .lookup_bucket(),  but create bucket if not already present
             * .second reports fractional weight (relative to unity) within bucket
             * that contains x.
             */
            std::pair<ProbabilityBucket*, double> establish_bucket(Domain const & x) {
                /* signed index.
                 *   ix>=0 => .lo_v[]
                 *   ix< 0 => .hi_v[]
                 */
                std::pair<int32_t, double> v = this->signed_bucket_index(x);

                int32_t ix = v.first;
                double fraction_wt = v.second;
                ProbabilityBucket * pw = nullptr;

                if (ix >= 0) {
                    size_t hi_ix = signed2hi(ix);

                    if (hi_ix >= this->hi_v_.size()) {
                        /* need to expand .hi_v[] */
                        this->hi_v_.resize(hi_ix+1);
                    }

                    pw = &(this->hi_v_[hi_ix]);
                } else {
                    size_t lo_ix = signed2lo(ix);

                    if (lo_ix >= this->lo_v_.size()) {
                        /* need to expand .lo_v[] */
                        this->lo_v_.resize(lo_ix+1);
                    }

                    pw = &(this->lo_v_[lo_ix]);
                }

                return std::make_pair(pw, fraction_wt);
            } /*establish_bucket*/

            void scale_bucket_by_signed_index(int32_t signed_ix, double k) {
                ProbabilityBucket * b = this->lookup_signed_bucket(signed_ix);

                if (b) {
                    b->scale_weight(k);

                    this->cdf_valid_flag_ = false;
                } else {
                    /* if bucket isn't represented,  then corresponding weight is zero,
                     * both before and after scaling.  In this special case
                     * representation didn't change -> don't invalidate cdf
                     */
                }
            } /*scale_bucket_by_signed_index*/

            /* scale probability assigned to bucket for domain value x, by factor k */
            void scale_bucket(Domain const & x, double k) {
                assert(k >= 0.0);

                std::pair<int32_t, double> v = this->signed_bucket_index(x);

                this->scale_bucket_by_signed_index(v.first, k);
            } /*scale_bucket*/

            /* scale probability weight for buckets in [lo, hi] by fn(p)
             * for a point p in each bucket,  however:
             * under no circumstances try to evaluate fn() outside [lo, hi]
             * (relevant if either lo/hi fall inside a bucket)
             */
            template<typename Function>
            void scale_interval(Domain const & lo,
                                Domain const & hi,
                                Function && fn)
                {
                    XO_SCOPE_DISABLED(lscope);

                    /* note: using inclusive upper index bounds here;
                     *       variying from idiomatic c++ style for symmetry
                     */

                    int32_t min_bucket_ix = lo2signed(this->lo_v_.size() - 1);
                    int32_t max_bucket_ix = hi2signed(this->hi_v_.size() - 1);

                    int32_t lo_ix = this->signed_bucket_index(lo).first;
                    int32_t hi_ix = this->signed_bucket_index(hi).first;

                    /* start_ix: lowest explicit bucket associating with [lo, hi) */
                    int32_t start_ix = std::max(min_bucket_ix, lo_ix + 1);
                    /* end_ix: highest explicit bucket associating with [lo, hi) */
                    int32_t end_ix = std::min(max_bucket_ix, hi_ix);

                    if (lscope.enabled()) {
                        lscope.log(xtag("min_bucket_ix", min_bucket_ix),
                                   xtag("max_bucket_ix", max_bucket_ix),
                                   xtag("lo_ix", lo_ix),
                                   xtag("hi_ix", hi_ix),
                                   xtag("start_ix", start_ix),
                                   xtag("end_ix", end_ix));
                    }

                    /* for endpoints:  avoid evaluating fn() outside [lo, hi] */
                    if (min_bucket_ix <= lo_ix) {
                        double lo_k = fn(lo);

                        if (lscope.enabled())
                            lscope.log("A", xtag("lo_ix", lo_ix), xtag("lo_k", lo_k));

                        this->scale_bucket_by_signed_index(lo_ix, lo_k);
                    }

                    for(int32_t ix = start_ix; ix <= end_ix; ++ix) {
                        double k = fn(this->bucket_mid(ix));

                        if (lscope.enabled())
                            lscope.log("B", xtag("ix", ix), xtag("k", k));

                        this->scale_bucket_by_signed_index(ix, k);
                    }

                    /* for endpoints: avoid evaluating fn() outside [lo, hi] */
                    if (hi_ix <= max_bucket_ix) {
                        double hi_k = fn(hi);

                        if (lscope.enabled())
                            lscope.log("C", xtag("hi_ix", hi_ix), xtag("hi_k", hi_k));

                        this->scale_bucket_by_signed_index(hi_ix, hi_k);
                    }
                } /*scale_interval*/

            template<typename Function>
            void scale_all(Function && fn)
                {
                    this->scale_interval(this->lo(), this->hi(), fn);
                } /*scale_all*/

            /* convenience: scale by scaled normal cdf.
             *
             * support use case where:
             * 1. explicit dist represents distribution of asset value;
             * 2. assume one party A to trade/order has noisy signal,
             *    with normally-distributed error around (unknown) true value s,
             *    with variance o^2;
             * 3. observe a trade/order at some price p,
             *    giving us one-sided information about A's knowledge in two scenarios:
             *    i.  A trades
             *    ii. A does not trade
             *
             * We will be applying Bayes' rule to update prior.
             * If buy(p) = {A buys},
             *   N(x) is cumulative normal distribution:
             *     N(x) -> 0 as x -> -oo;
             *     N(x) -> 1 as x -> +oo;
             *
             *                        -1/2        2
             *     (d/dx)N(x) = (2.pi)    . exp(-x /2)
             * then:
             *                     P{s=s'}
             *   P{s=s'|buy(p)} = ---------.P{buy(p)|s=s'}
             *                    P{buy(p)}
             *
             *                     P{s=s'}
             *                  = ---------.N[(1/o)(s'-p)],
             *                    P{buy(p)}
             *
             * In this scenario, P{s=s'} is our prior,  represented by distribution *this.
             * The unconditional probability P{buy(p)} is not a function of s',  so:
             *
             *                /
             *                |
             *    P{buy(p)} = | P{s=s'} . N[(1/o)(s'-p)] . ds'
             *                |
             *                /
             *
             * in other words can scale explicit prior *this by N[(1/o)(s'-p)] then renormalize
             * so total probability weight is 1.
             *
             * Conversely,  if event is {A sells}, similar argument leads to scaling
             * by N[(1/o)(p-s')]
             *
             * sign.  scale by N[(1/sigma).sign.(x-mean)]
             */
            void scale_by_normal_cdf(int sign, Domain const & mean, Domain const & sigma) {
                auto fn([sign, mean, sigma](Domain const & s) {
                    return Normal::cdf_impl(sign * (s - mean) / sigma);
                });

                this->scale_all(fn);
            } /*scale_by_normal_cdf*/

            // ----- inherited from Distribution<Domain> -----

            /* note: marked const;  actually "logically const" */
            virtual double cdf(Domain const & x) const override {
                /* .cdf() is slow here,  because partial sums aren't stored
                 * --> have to sum over O(n) buckets
                 */
                {
                    ExplicitDist<Domain> * self = const_cast<ExplicitDist<Domain> *>(this);

                    self->check_renormalize();
                }

                std::pair<ProbabilityBucket const *, double> v = this->lookup_bucket(x);

                ProbabilityBucket const * b = v.first;
                double fraction_wt = v.second;

                if (b) {
                    /* for bucket that x belongs to,   treat as uniformly distributed
                     *   fraction_wt = 1.0 -> 100% of b -> b.cdf()
                     *   fraction_wt = 0.0 ->   0% of b -> b.cdf() - b.weight()
                     */
                    return b->cdf() + (fraction_wt - 1.0) * b->weight();
                } else {
                    if (x >= this->ref_value_) {
                        /* x falls above farthest .hi_v[] */
                        return 1.0;
                    } else {
                        /* x falls below farthest .lo_v[] */
                        return 0.0;
                    }
                }
            } /*cdf*/

            /* O(n).  restores .cdf_valid_flag */
            void renormalize() {
                /* [1] compute sum of probability weights
                 * [2] rescale all buckets so sum is 1.
                 * [3] restore .cdf_valid_flag
                 */

                /* [1] */
                double lo_sum_prob = 0.0;
                for (ProbabilityBucket & b : this->lo_v_) {
                    assert(lo_sum_prob >= 0.0);
                    lo_sum_prob += b.weight();
                }

                double hi_sum_prob = 0.0;
                for (ProbabilityBucket & b : this->hi_v_) {
                    assert(hi_sum_prob >= 0.0);
                    hi_sum_prob += b.weight();
                }

                assert(lo_sum_prob + hi_sum_prob > 0.0);

                /* [2] */
                double renorm_factor = 1.0 / (lo_sum_prob + hi_sum_prob);

                {
                    double lo_tail = lo_sum_prob;

                    /* iterate over buckets < .ref_value,  in /descending/ domain order */
                    for (ProbabilityBucket & b : this->lo_v_) {
                        b.scale_weight(renorm_factor);
                        b.assign_cdf(lo_tail);

                        /* reduce tail after assign cdf to b,  since
                         * lo_tail includes b's probability weight
                         */
                        lo_tail -= b.weight();

                        /* numerical roundoff can cause this */
                        if (lo_tail < 0.0)
                            lo_tail = 0.0;
                    }
                }

                {
                    double cum_prob = lo_sum_prob;

                    /* iterate over buckets >= .refvalue,  in /ascending/ domain order */
                    for (ProbabilityBucket & b : this->hi_v_) {
                        b.scale_weight(renorm_factor);

                        /* increase cum_prob before assign cdf to b,
                         * since b.cdf should include b's probability weight
                         */
                        cum_prob += b.weight();

                        /* numerical roundoff can cause this */
                        if (cum_prob >= 1.0)
                            cum_prob = 1.0;

                        b.assign_cdf(cum_prob);
                    }
                }

                /* [3] */
                this->cdf_valid_flag_ = true;
            } /*renormalize*/

            void check_renormalize() {
                if (!this->cdf_valid_flag_) {
                    this->renormalize();
                }
            } /*check_renormalize*/

            void display(std::ostream & os) const {
                os << "<ExplicitDist";
                os << xtag("cdf_valid_flag", this->cdf_valid_flag_);
                os << xtag("bucket_dx", this->bucket_dx_);
                os << xtag("ref_value", this->ref_value_);
                os << xtag("lz", this->lo_v_.size());
                os << xtag("hz", this->hi_v_.size());
                os << xtag("lo_v", this->lo_v_);
                os << xtag("hi_v", this->hi_v_);
                os << ">";
            } /*display*/

            std::string display_string() const { return xo::tostr(*this); }

        private:
            ExplicitDist(Domain bucket_dx, Domain ref_value)
                : cdf_valid_flag_{true},
                  bucket_dx_{bucket_dx},
                  ref_value_{ref_value},
                  lo_v_{},
                  hi_v_{1}
                {
                    assert(bucket_dx_ > 0.0);

                    /* must have at least one bucket,  since need total probability weight = 1 */
                    this->hi_v_[0].assign_weight(1.0);
                    this->hi_v_[0].assign_cdf(1.0);
                }

            ExplicitDist(size_t n, Domain bucket_dx, Domain ref_value)
                : cdf_valid_flag_{true},
                  bucket_dx_{bucket_dx},
                  ref_value_{ref_value},
                  lo_v_{},
                  hi_v_{n}
                {
                    assert(bucket_dx > 0.0);
                    /* must have at least one bucket,  since need total probability weight = 1 */
                    assert(n > 0);

                    double w = 1.0 / n;

                    for(size_t i = 0; i<n; ++i) {
                        this->hi_v_[i].assign_weight(w);
                        this->hi_v_[i].assign_cdf((i+1)*w);
                    }
                } /*ctor*/

        private:
            /* with lz=.lo_v.size(), hz=hi_v.size():
             *
             *    .ref_value - .bucket_dx * lz
             *    |                 .ref_value
             *    |                 | .hi_v[0]     .hi_v[hz-1]
             *    v                 v v            v
             *                      |
             *    +--+--+     +--+--|--+--+      +--+
             *    |  |  | ... |  |  |  |  |  ... |  |
             *    +--+--+     +--+--|--+--+      +--+
             *                      |
             *      ^              ^                ^
             *      .lo_v[lz-1]    .lo_v[0]         .ref_value + .bucket_dx * hz
             *
             */

            /* .cdf_valid_flag:
             *   -> false whenever .scale_bucket() runs.
             *   -> true whenever .renormalize() runs.
             *
             * if false,  then *this is 'not a probability distribution':
             *   Sum b[i].weight != 1.0   (summing over probability buckets .lo_v[], .hi_v[])
             *    i
             */
            bool cdf_valid_flag_ = true;

            /* width of each bucket */
            Domain bucket_dx_;

            /* natural value here would be 0.
             * use .pos_v[] for values >= .ref_value
             * use .neg_v[] for values <  .ref_value
             */
            Domain ref_value_;

            /* buckets for domain values < .ref_value
             *
             * with dx = .bucket_dx
             * .lo_v[i] represents weights in range
             *   [.ref_value - (i+1) * dx, .ref_value - i * dx)
             */
            WeightVector lo_v_;

            /* buckets for domain values > .ref_value
             *
             * with dx = .bucket_dx
             * .hi_v[i] represents weights in range
             *   [.ref_value + i * dx, .ref_value + (i+1) * dx))
             */
            WeightVector hi_v_;

        }; /*ExplicitDist*/

        template<typename Domain>
        inline std::ostream &
        operator<<(std::ostream & os, ExplicitDist<Domain> const & x) {
            x.display(os);
            return os;
        } /*operator<<*/
    } /*namespace distribution*/
} /*namespace xo*/

/* end ExplicitDist.hpp */
