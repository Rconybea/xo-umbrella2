/* @file KalmanFilterSvc.cpp */

#include "KalmanFilterSvc.hpp"
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */
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
    KalmanFilterSvc::pretty(xo::pp::PpSink & sink) const
    {
        using xo::pp::field;

        const auto nm = this->name();
        const auto n_in = this->n_in_ev();
        const auto n_q = this->n_queued_out_ev();
        const auto n_out = this->n_out_ev();

        sink.pretty_struct("KalmanFilterSvc",
                           field("name", nm),
                           field("n_in_ev", n_in),
                           field("n_queued_out_ev", n_q),
                           field("n_out_ev", n_out));
    }
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSvc.cpp */
