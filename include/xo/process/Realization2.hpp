/* file Realization2.hpp
 *
 * author: Roland Conybeare, Nov 2022
 */

#pragma once

#include "AbstractRealization.hpp"
#include "xo/reflect/Reflect.hpp"
//#include "time/Time.hpp"

namespace xo {
    namespace process {
        template<typename T>
        class Realization2 : public AbstractRealization {
        }; /*Realization2*/

        /* Rstate: state needed to trace unfolding of a process
         * realization;  will be process-specific.
         *
         * Pattern like:
         *   StochasticProcess p
         *   Rstate rs
         *   Realization2 rz
         *
         *   +----+    +----+
         *   | rz +----| rs |
         *   +--+-+    +----+
         *      |        ^
         *   +----+      |
         *   |  p | <----/
         *   +----+
         *
         *   rz owns rs,  sends it to p to be modified as p needs
         *   p knows type Rstate,  initially creates it
         *   therefore also knows how to create its own realizations
         *
         *
         * Require:
         * - Process -isa-> Realizable2Process<T, Rstate>
         */
        template<typename T, typename Rstate, typename Process>
        class ProcessRealization2 : public Realization2<T> {
        public:
            using TaggedRcptr = reflect::TaggedRcptr;
            using nanos = xo::time::nanos;

        public:
            ProcessRealization2(Rstate const & rstate, rp<Process> const & process)
                : rstate_{rstate}, process_{process} {}
            ProcessRealization2(Rstate && rstate, rp<Process> const & process)
                : rstate_{std::move(rstate)}, process_{process} {}

            Rstate const & rstate() const { return rstate_; }
            rp<Process> const & process() const { return process_; }

            /* sample process at point .rstate.tk + dt
             * Require:
             * - dt >= 0
             */
            void advance_dt(nanos dt) {
                this->process_->rstate_sample_inplace(dt, &(this->rstate_));
            } /*advance_dt*/

            // ----- inherited from AbstractRealization -----

            virtual rp<AbstractStochasticProcess> stochastic_process() const override {
                return process_;
            } /*stochastic_process*/

            // ----- inherited from SelfTagging -----

            virtual TaggedRcptr self_tp() override { return reflect::Reflect::make_rctp(this); }

        private:
            /* realization state
             * this type is determined by .process;
             * sufficient state to develop faithful realization
             */
            Rstate rstate_;
            /* process (set of paths + probability measure);
             * *this coordinates with .process to constructively samples one such path
             */
            rp<Process> process_;
        }; /*ProcessRealization2*/
    } /*namespace process*/

} /*namespace xo*/


/* end Realization2.hpp */
