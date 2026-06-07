/* file RealizationState.hpp
 *
 * author: Roland Conybeare, Nov 2022
 */

#pragma once

#include <utility>

/* opaque type representing state of an unfolding
 * realization, for a StochasticProcess.
 * Needs runtime polymorphism here so we can stack states
 * e.g. to represent realization state for a process
 * defined by transformation of another process.
 * For example see ExpProcess.
 * For now we don't refcount these;  expect each process-realization
 * to create its own stack,  managed with unique_ptr<>
 *
 * See also:
 * - ProcessRealization2
 * - Realizable2Process
 */
class AbstractRealizationState {
public:
  AbstractRealizationState() = default;

  virtual ~AbstractRealizationState() = default;
}; /*RealizationState*/

template<typename Rstate>
class RealizationState : public AbstractRealizationState {
public:
  RealizationState(Rstate const & rs) : rstate_{rs} {}
  RealizationState(Rstate && rs) : rstate_{std::move(rs)} {}

  Rstate * p_rstate() { return &rstate_; }

private:
  Rstate rstate_;
}; /*RealizationState*/

/* end RealizationState.hpp */
