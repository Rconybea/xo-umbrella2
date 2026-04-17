/** @file GcosTestutil.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/gc/GCObjectStore.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/arena/DArena.hpp>
#include <xo/randomgen/xoshiro256.hpp>

namespace ut {
    using xo::facet::obj;

    enum class TestGraphType {
        /* list cell pointing to itself */
        selfcycle,
        /* random object graph */
        random,
    };

    /** record capturing some stats for a (randomly created) gc-aware object **/
    struct Recd {
        using AGCObject = xo::mm::AGCObject;
        using typeseq = xo::reflect::typeseq;

        Recd() = default;
        Recd(obj<AGCObject> value, uint32_t z, typeseq tseq)
            : gco_{value}, alloc_z_{z}, tseq_{tseq} {}

        // random gc-aware value
        obj<AGCObject> gco_;
        // expected allocation size (lower bound)
        uint32_t alloc_z_ = 0;
        // representation
        typeseq tseq_;
    };

    struct GcosTestutil {
        using GCObjectStore = xo::mm::GCObjectStore;
        using DArena = xo::mm::DArena;
        using xoshiro256ss = xo::rng::xoshiro256ss;

        static void
        selfcycle_object_graph(std::vector<Recd> * p_v1,
                               GCObjectStore * p_gcos,
                               std::vector<Recd> * p_v2,
                               DArena * arena2);

        static void
        random_object_graph(uint32_t n_new_obj,
                            uint32_t n_assign,
                            xoshiro256ss * p_rgen,
                            std::vector<Recd> * p_v,
                            GCObjectStore * p_gcos,
                            std::vector<Recd> * p_v2,
                            DArena * p_arena2);

        static void
        gcos_install_test_types(bool do_type_registration,
                                GCObjectStore * p_gcos);

        static void
        gcos_verify_arena_partitioning(uint32_t n_gen,
                                       size_t gc_size,
                                       const GCObjectStore & gcos);

        static void
        gcos_verify_vacant(uint32_t n_gen,
                           size_t gc_size,
                           const GCObjectStore & gcos);

        static void
        gcos_construct_ab_object_graphs(TestGraphType obj_graph_type,
                                        uint32_t n_i0_test_obj,
                                        uint32_t n_i0_test_assign,
                                        uint32_t n_i1_test_obj,
                                        uint32_t n_i1_test_assign,
                                        GCObjectStore * p_gcos,
                                        DArena * p_arena2,
                                        uint32_t loop_index,
                                        std::vector<Recd> * p_x1_v,
                                        std::vector<Recd> * p_x2_v,
                                        xoshiro256ss * p_rgen);
    };
} /*namespace ut*/

/* end GcosTestutil.hpp */
