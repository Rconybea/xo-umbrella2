/** @file GcosTestutil.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/gc/MutationLogStore.hpp>
#include <xo/gc/GCObjectStore.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Generation.hpp>
#include <xo/arena/DArena.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <array>

namespace ut {
    using xo::mm::Generation;
    using xo::facet::obj;

    /** specify a step in scripted sequence
     **/
    struct Step {
        enum class Cmd {
            /** sentinel for end of sequence **/
            sentinel,
            /** refer to nil DList **/
            make_nil,
            /** allocate DList w/ head x1_v[arg0_ix_], rest x1_v[arg1_ix_] **/
            make_cons,
            /** allocate a boolean **/
            make_bool,
            /** allocate an integer **/
            make_int,
            /** assign a top-level value from one slot to another **/
            assign_root,
            /** modify the head of a list x1_v[arg0_ix_]; replace with x1_v[arg1_ix_] **/
            assign_head,

        };

        Step(Cmd cmd, uint32_t arg0, uint32_t arg1)
        : cmd_{cmd}, arg0_ix_{arg0}, arg1_ix_{arg1} {}

        bool is_sentinel() const { return cmd_ == Cmd::sentinel; }
        bool is_command() const { return cmd_ != Cmd::sentinel; }

        Cmd cmd_;
        /** arg0 object index (index into x1_v[])
         *
         * when cmd_ is make_bool:
         *   0 -> false, 1 -> true
         * when cmd_ is make_int:
         *   value of integer
         * when cmd_ is assign_root:
         *   index of lhs value to replace
         **/
        uint32_t arg0_ix_ = 0;
        /** arg1 object index (index into x1_v[])
         *
         *  when cmd_ is assign_root:
         *    index of rhs value to assign
         **/
        uint32_t arg1_ix_ = 0;
    };

    /** a phase comprises:
     *  1. start with {gcos,mls} in known + valid state.
     *  2. perform a sequence of commands
     *     (in general a mix of allocs and mutations)
     *     command sequence in @ref cmd_seq_, null-terminated
     *  3. verify mlog state after sequence
     *  4. run instrumented collection phase
     *     4a. swap roles (i.e. from- and to- spaces)
     *     4b. move roots, see gcos_move_roots_and_verify()
     *     4c. update mutation log, see forward_mutation_log()
     *     4d. cleanup (reset from- spaces)
     *  5. re-verify {gcos,mls} in valid state
     **/
    struct Phase {
        bool is_sentinel() const noexcept { return lo_ix_ == -1; }

        /** Command sequence for this phase.
         *  See TestSequence.cmd_seq_
         *  Phase comprises commands cmd_seq_[ix] for lo_ix <= ix < hi_ix
         **/
        int32_t lo_ix_ = -1;
        int32_t hi_ix_ = -1;
        /** expected number of new entries in
         *  to-space mutation log after executing @ref cmd_seq_
         **/
        std::array<uint32_t, xo::mm::c_max_generation - 1> mlog_new_z_;
    };

    struct TestSequence {
        bool is_sentinel() const noexcept { return cmd_seq_ == nullptr; }

        /** shared null-terminated command sequence.
         *  references are taken relative to cmd_seq_[0].
         *  A step
         *    {make_cons, x, y} -> make a cons cell.
         *                         head from value ~ cmd_seq_[x]
         *                         rest from value ~ cmd_seq_[y]
         *
         **/
        Step * cmd_seq_ = nullptr;

        /** array of phases.
         *  One gc cycle per phase.
         *  Sentinel phase has {lo_ix_ = -1, hi_ix_ = -1};
         **/
        Phase * phases_ = nullptr;
    };

    enum class TestGraphType {
        /* spelled out sequence of Steps */
        fixed,
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
        using MutationLogStore = xo::mm::MutationLogStore;
        using GCObjectStore = xo::mm::GCObjectStore;
        using AGCObject = xo::mm::AGCObject;
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
                            bool debug_flag,
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

        /** sequence of steps. if non-null, ends with step s: s.cmd_ == Step::Cmd::Sentinel
         *
         *  @p p_cmd_seq pointer to null-terminated array of Step[] arrays
         **/
        static void
        gcos_construct_ab_object_graphs(TestSequence test_seq,
                                        TestGraphType obj_graph_type,
                                        uint32_t n_i0_test_obj,
                                        uint32_t n_i0_test_assign,
                                        uint32_t n_i1_test_obj,
                                        uint32_t n_i1_test_assign,
                                        bool debug_flag,
                                        MutationLogStore * p_mls,
                                        GCObjectStore * p_gcos,
                                        DArena * p_arena2,
                                        uint32_t loop_index,
                                        std::vector<Recd> * p_x1_v,
                                        std::vector<Recd> * p_x2_v,
                                        xoshiro256ss * p_rgen);

        /** Invoke built-in consistency verification for @p *p_gcos.
         **/
        static void
        gcos_verify_consistency(GCObjectStore * p_gcos);

        static void
        gcos_verify_ab_equivalence(const std::vector<Recd> & x1_v,
                                   const std::vector<Recd> & x2_v);

        /** verify reasonable alloc info values.
         *  object store has been subject to @p loop_index
         *  collection cycles
         **/
        static void
        gcos_verify_allocinfo(const GCObjectStore & gcos,
                              uint32_t loop_index,
                              const std::vector<Recd> & x1_v);

        static void
        gcos_verify_gen0_only_allocated(uint32_t n_gen,
                                        const GCObjectStore & gcos,
                                        uint32_t loop_index,
                                        const std::vector<Recd> & x1_v);

        static void
        gcos_verify_gen0_fromspace_only_allocated(uint32_t n_gen,
                                                  const GCObjectStore & gcos,
                                                  uint32_t loop_index,
                                                  Generation upto,
                                                  const std::vector<Recd> & x1_v);
        static void
        gcos_move_roots_and_verify(bool do_type_registration,
                                   GCObjectStore * p_gcos,
                                   Generation upto,
                                   const std::vector<Recd> & x1_v,
                                   const std::vector<Recd> & x2_v,
                                   bool debug_flag);

        static void
        gcos_verify_forwarding(const GCObjectStore & gcos,
                               Generation upto,
                               const Recd & x1,
                               obj<AGCObject> x1_gco);

        static void
        gcos_verify_forwarding_destination(const GCObjectStore & gcos,
                                           const Recd & x1,
                                           obj<AGCObject> x1p_gco);

        static void
        gcos_verify_forwarded_ab_equivalence(obj<AGCObject> x1p_gco,
                                             obj<AGCObject> x2_gco);
    };
} /*namespace ut*/

/* end GcosTestutil.hpp */
