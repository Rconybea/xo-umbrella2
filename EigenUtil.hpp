/* file EigenUtil.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

namespace xo {
  namespace json { class PrintJson; }

  namespace eigen {
    class EigenUtil {
    public:
      /* reflection for
       *   Eigen::VectorXd
       *   VectorXb (= Eigen::Array<bool, Eigen::Dynamic, 1>; by analogy with Eigen::VectorXd)
       */
      static void reflect_eigen();

      /* json printers for:
       *   Eigen::VectorXd
       *   Eigen::MatrixXd
       */
      static void provide_json_printers(json::PrintJson * pjson);
    }; /*EigenUtil*/
  } /*namespace eigen*/
} /*namespace xo*/

/* end EigenUtil.hpp */
