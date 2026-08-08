/* @file KalmanFilterSvc.cpp */

#include "KalmanFilterSvc.hpp"
/* xo::scope / xo::xtag -- were arriving via xo-reactor headers,
 * which are now ppsink-only.
 */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */

namespace xo {
  using xo::rp;

  namespace kalman {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * *ambiguous* with legacy xo::xtag (still visible via headers that
         * have not migrated) rather than shadowing it.
         */
      using xo::pp::scope;
      using xo::pp::xtag;

    rp<KalmanFilterSvc>
    KalmanFilterSvc::make(KalmanFilterSpec spec)
    {
      return new KalmanFilterSvc(std::move(spec));
    } /*make*/

    KalmanFilterSvc::KalmanFilterSvc(KalmanFilterSpec spec)
      : filter_{std::move(spec)}
    {}

    void
    KalmanFilterSvc::notify_ev(rp<KalmanFilterInput> const & input_kp1)
    {
      this->filter_.notify_input(input_kp1);

      ++(this->n_in_ev_);
      this->notify_secondary_event(this->filter_.state_ext());
    } /*notify_input*/

    void
    KalmanFilterSvc::display(std::ostream & os) const
    {
      os << "<KalmanFilterSvc"
     << xtag("name", this->name())
     << xtag("n_in_ev", this->n_in_ev())
     << xtag("n_queued_out_ev", this->n_queued_out_ev())
     << xtag("n_out_ev", this->n_out_ev())
    //<< xtag("filter", this->filter_)
     << ">";
    } /*display*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSvc.cpp */
