/* @file xoshiro256.hpp */

#pragma once

#include "engine_concept.hpp"
#include <iostream>
#include <array>
#include <limits>
#include <cstdint>

namespace xo {
    namespace rng {

        /* engine for producing 64-bit random numbers
         *
         * see https:/en.wikipedia.org/wiki/Xorshift#xoshiro256**
         *
         * - satisfies c++ UniformRandomBitGenerator
         * - satisfies c++
         *
         * Note:  zero seed --> constant output sequence {0, 0, 0, ...}
         */
        class xoshiro256ss {
        public:
            using result_type = std::uint64_t;
            using seed_type = std::array<std::uint64_t, 4>;

        public:
            /* null state -- generates constant stream of 0 bits */
            xoshiro256ss() : xoshiro256ss(0) {}
            /* copy ctor */
            xoshiro256ss(xoshiro256ss const & x) = default;
            xoshiro256ss(seed_type const & seed) : s_(seed) {}

            /* fallback version -- deprecated */
            xoshiro256ss(std::uint64_t seed)
                {
                    this->s_[0] = 0;
                    this->s_[1] = seed;
                    this->s_[2] = 0;
                    this->s_[3] = 0;

                    generate();
                }

            static constexpr std::uint64_t min() { return 0; }
            static constexpr std::uint64_t max() { return std::numeric_limits<std::uint64_t>::max(); }

            static std::uint64_t rol64(std::uint64_t x, std::int64_t k)
                {
                    return (x << k) | (x >> (64 - k));
                }

            static bool equal(xoshiro256ss const & x, xoshiro256ss const & y) {
                return ((x.s_[0] == y.s_[0])
                        && (x.s_[1] == y.s_[1])
                        && (x.s_[2] == y.s_[2])
                        && (x.s_[3] == y.s_[3]));
            }

            /* puts generator into null state */
            void seed() { *this = xoshiro256ss(); }
            void seed(std::uint64_t s) { *this = xoshiro256ss{s}; }
            /* e.g. used with std::seed_seq<> */
            template <typename SeedSeq>
            void seed(SeedSeq & sseq) {
                sseq.generate(s_.begin(), s_.end());
            }

            std::uint64_t generate() {
                std::array<std::uint64_t, 4> & s = (this->s_);
                std::uint64_t const result = rol64(s[1] * 5, 7) * 9;
                std::uint64_t const t = s[1] << 17;

                s[2] ^= s[0];
                s[3] ^= s[1];
                s[1] ^= s[2];
                s[0] ^= s[3];

                s[2] ^= t;
                s[3] = rol64(s[3], 45);

                return result;
            } /*generate*/

            /* advance to same state as obtained from z calls to .generate().   O(z) !
             * usually better to use jump().
             *
             * providing .discard() to satisfy c++ named requirement _RandomNumberEngine_
             */
            void discard(std::uint64_t z) {
                for (std::uint64_t i=0; i<z; ++i)
                    this->generate();
            }

            /* equivalent to .discard(2^128),  but uses O(1) time
             *
             * (may use in multithreaded program to get determinsitic non-overlapping random sequences)
             */
            void jump() {
                std::array<std::uint64_t, 4> const s_jump_v
                    = {{0x180ec6d33cfd0aba,
                        0xd5a61266f0c9392c,
                        0xa9582618e03fc9aa,
                        0x39abdc4529b1661c}};

                std::array<std::uint64_t, 4> & s = (this->s_);

                std::uint64_t s0 = 0;
                std::uint64_t s1 = 0;
                std::uint64_t s2 = 0;
                std::uint64_t s3 = 0;
                for (std::uint32_t i = 0; i < s_jump_v.size(); ++i) {
                    for (std::uint32_t bit = 0; bit < 64; ++bit) {
                        if (s_jump_v[i] & 1UL << bit) {
                            s0 ^= s[0];
                            s1 ^= s[1];
                            s2 ^= s[2];
                            s3 ^= s[3];
                        }
                        this->generate();
                    }
                }

                s[0] = s0;
                s[1] = s1;
                s[2] = s2;
                s[3] = s3;
            } /*jump*/

            /* inverse of .load() */
            void print(std::ostream & os) const {
                os << "<xoshiro256ss " << s_[0] << " " << s_[1] << " " << s_[2] << " " << s_[3] << ">";
            }

            /* inverse of .print() */
            void load(std::istream & is) {
                std::string header, trailer;
                std::array<std::uint64_t, 4> sv;

                is >> header >> sv[0] >> sv[1] >> sv[2] >> sv[3] >> trailer;

                if ((header != "<xoshiro256ss") || trailer != ">")
                    throw std::runtime_error("xoshiro256ss.load: bad input format, expecting input like <xoshiro256ss $s0 $s1 $s2 $s3>");

                this->s_ = sv;
            } /*load*/

            std::uint64_t operator()() { return generate(); }

        private:
            /* state */
            std::array<std::uint64_t, 4> s_;
        }; /*xoshiro256ss*/

        inline bool operator==(xoshiro256ss const & x, xoshiro256ss const & y) {
            return xoshiro256ss::equal(x, y);
        }

        inline bool operator!=(xoshiro256ss const & x, xoshiro256ss const & y) {
            return !xoshiro256ss::equal(x, y);
        }

        static_assert(engine_concept<xoshiro256ss>);

    } /*namespace rng*/
} /*namespace xo*/

/* end xoshiro256.hpp */
