/* file Realizable2Process.hpp
 *
 * author: Roland Conybeare, Nov 2022
 */

#pragma once

#include "StochasticProcess.hpp"
#include "Realization2.hpp"
#include "RealizationState.hpp"

namespace xo {
  namespace process {
    /* a stochastic process p that interacts with a Realization2<T, Rstate>
     * This means:
     * - p defines state (Rstate) sufficient to constructively unroll/unfold
     *   one of its own paths
     * - p provides methods to implement such unfolding:
     *   - .make_realization() :: Realization2<T>
     *      create new realization of p.  
     *   - .rstate_sample(t1,rs0) :: time x Rstate -> Rstate
     *      given runstate rs0 representing process state at some time t0,
     *      sample process at time t1, with t0<=t1
     * - in general can only sample process at a bounded set of points;
     *   sometimes useful to be able to generate samples consistently in
     *   non-monotonically-increasing time order.  Algorithm to do this available
     *   for some,  but not all p
     *   - .rstate_insample(t1,rs0,rs2) :: time x Rstate x Rstate -> Rstate
     *     given runstates rs0, rs2 representing process state at two times t0<t2,
     *     sample process at time t1, with t0<=t1<=t2
     *     
     * Require:
     * - Rstate.last() :: (time x T)  last process sample represented by Rstate
     *                    if p is a markov process, this is also sufficient to drive
     *                    process unfolding
     * - Rstate.tn() :: time
     *   location in time represented by a particular Rstate (same as .last().first)
     */
    template<typename T>
    class Realizable2Process : public StochasticProcess<T> {
    public:
      virtual ref::rp<Realization2<T>> make_realization() = 0;
      /* make_rstate() will be used to establish nested state when a process is used
       * as input to a transforming process (ex: ExpProcess).
       * in that context the outer process' realization state will
       * need to hold an abstract pointer to nested process' realization state,
       * and use this method to establish that state.
       */
      virtual std::unique_ptr<AbstractRealizationState> make_rstate() = 0;
      // Rstate rstate_init() const;
      // void rstate_sample_implace(utc_nanos t1, Rstate * p_rs0 const;
    }; /*Realizable2Process*/
  
  } /*namespace process*/
} /*namespace xo*/


/* end Realizable2Process.hpp */
