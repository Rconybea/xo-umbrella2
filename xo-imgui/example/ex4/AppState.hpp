/* AppState.hpp */

#pragma once

#include "GcStateDescription.hpp"
#include "GcCopyDetail.hpp"
#include "xo/alloc/GC.hpp"
#include "xo/alloc/Object.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include "xo/randomgen/random_seed.hpp"
#include <cstdint>

struct AppState {
    using GC = xo::gc::GC;
    using generation = xo::gc::generation;
    using Object = xo::Object;
    using xoshiro256ss = xo::rng::xoshiro256ss;
    template <typename Rng>
    using Seed = xo::rng::Seed<Rng>;

public:
    AppState();

    std::size_t nursery_tospace_scale() const;
    std::size_t tenured_tospace_scale() const;
    GcStateDescription snapshot_gc_state() const;

    void generate_random_mutation();
    void generate_random_mutations();

public:
    int alloc_per_cycle_ = 1;
    /** if gc triggered, remembers which whether incremental or full **/
    generation upto_ = generation::nursery;
    xo::up<GC> gc_;
    std::size_t next_int_ = 0;
    std::size_t next_root_ = 0;
    std::vector<xo::gp<Object>> gc_root_v_{100};
    Seed<xoshiro256ss> seed_;
    xoshiro256ss rng_{seed_};
    /** remember details for each object copied by GC, so we can animate **/
    std::vector<GcCopyDetail> copy_detail_v_;
    /** max offset for destination, given copied to nursery **/
    std::size_t copy_detail_max_nursery_dest_offset_ = 0;
    std::size_t copy_detail_nursery_dest_size_ = 0;
    std::size_t copy_detail_max_tenured_dest_offset_ = 0;
    std::size_t copy_detail_tenured_dest_size_ = 0;
};

/* AppState.cpp */
