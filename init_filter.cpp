/* file init_filter.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "init_filter.hpp"
#include "reactor/init_reactor.hpp"
#include "KalmanFilterState.hpp"
#include "EigenUtil.hpp"
#include "printjson/PrintJson.hpp"

namespace xo {
  using xo::kalman::KalmanFilterInput;
  using xo::kalman::KalmanFilterTransition;
  using xo::kalman::KalmanFilterState;
  using xo::kalman::KalmanFilterStateExt;
  using xo::eigen::EigenUtil;
  using xo::json::PrintJsonSingleton;
  using xo::json::PrintJson;

  void
  InitSubsys<S_filter_tag>::init()
  {
    PrintJson * pjson = PrintJsonSingleton::instance().get();

    EigenUtil::reflect_eigen();
    EigenUtil::provide_json_printers(pjson);

    KalmanFilterInput::reflect_self();
    KalmanFilterTransition::reflect_self();
    KalmanFilterState::reflect_self();
    KalmanFilterStateExt::reflect_self();

  } /*init*/

  InitEvidence
  InitSubsys<S_filter_tag>::require()
  {
    InitEvidence retval;

    /* subsystem dependencies for filter/ */
    retval ^= InitSubsys<S_reactor_tag>::require();

    /* filter/'s own initialization code */
    retval ^= Subsystem::provide<S_filter_tag>("filter", &init);

    return retval;
  } /*require*/
} /*namespace xo*/

/* end init_filter.cpp */
