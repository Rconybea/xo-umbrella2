/* @file KalmanFilterSvc.cpp */

#include "KalmanFilterSvc.hpp"

namespace xo {
  using xo::rp;
  using xo::scope;
  using xo::xtag;

  namespace kalman {
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
