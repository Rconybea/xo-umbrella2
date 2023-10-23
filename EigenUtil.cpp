/* file EigenUtil.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "EigenUtil.hpp"
#include "printjson/PrintJson.hpp"
#include "reflect/Reflect.hpp"
#include <Eigen/Dense>
#include <memory>
#include <cstdint>
#include <cassert>

namespace xo {
  using xo::json::PrintJson;
  using xo::json::JsonPrinter;
  using xo::reflect::Reflect;
  using xo::reflect::TypeDescr;
  using VectorXb = Eigen::Array<bool, Eigen::Dynamic, 1>;
  using Eigen::VectorXd;
  using Eigen::MatrixXd;

#ifdef NOT_YET
  namespace reflect {
    template<typename ElementType>
    using EigenVectorX_Tdx = xo::reflect::StlVectorTdx<Eigen::Array<ElementType,
								    Eigen::Dynamic,
								    1>>;
    
    /* probably need this to appear before decl for class xo::reflect::Reflect */
    template<typename ElementType>
    class EstablishTdx<Eigen::Array<ElementType, Eigen::Dynamic, 1>> {
    public:
      static std::unique_ptr<TypeDescrExtra> make() {
	return EigenVectorX_Tdx<ElementType>::make();
      } /*make*/
    }; /*EstablishTdx*/
  } /*reflect*/
#endif
  
  namespace eigen {

    namespace {
      /* prints a VectorXd as json,  in the obvious format, e.g.
       *   [1,2,3]
       */
      template<typename EigenVectorType>
      class EigenVectorJsonPrinter : public JsonPrinter {
      public:
	EigenVectorJsonPrinter(PrintJson const * pjson) : JsonPrinter(pjson) {}

	virtual void print_json(TaggedPtr tp,
				std::ostream * p_os) const override
	{
	  EigenVectorType * pv = this->check_recover_native<EigenVectorType>(tp, p_os);

	  if (pv) {
            /* EigenVectorType (VectorXb, VectorXd, ..)
	     * is reflected as atomic for now, out of expedience.
	     *
             * as soon as we reflect as mt_vector,  will not need this helper.
             */
            *p_os << "[";

            for (std::uint32_t i = 0, n = pv->size(); i < n; ++i) {
              if (i > 0)
                *p_os << ",";

		/* note: need to dispatch via json printer for vector elements,
		 *       to get special treatment for non-finite values
		 */
	      this->pjson()->print((*pv)[i], p_os);
	      //*p_os << jsonp((*pv)[i], this->pjson());
            }

            *p_os << "]";
	  }
	} /*print_json*/
      }; /*EigenVectorJsonPrinter*/

      /* prints a MatrixXd as json,   in row-major format, e.g.
       *   [[1,2,3], [4,5,6], [7,8,9]]
       */
      class MatrixXdJsonPrinter : public JsonPrinter {
      public:
	MatrixXdJsonPrinter(PrintJson const * pjson) : JsonPrinter(pjson) {}

	virtual void print_json(TaggedPtr tp,
				std::ostream * p_os) const override
	{
	  MatrixXd * pm = this->check_recover_native<MatrixXd>(tp, p_os);

	  if (pm) {
	    /* MatrixXd is reflected as atomic for now, out of expedience */
	    *p_os << "[";

	    for(std::uint32_t i=0, m=pm->rows(); i<m; ++i) {
	      if (i > 0)
		*p_os << ", ";
	      *p_os << "[";
	      for(std::uint32_t j=0, n=pm->cols(); j<n; ++j) {
		if (j > 0)
		  *p_os << ",";

		/* note: need to dispatch via json printer for matrix elements,
		 *       to get special treatment for non-finite values
		 */
		this->pjson()->print((*pm)(i, j), p_os);
		//*p_os << jsonp((*pm)(i, j), this->pjson());
	      }
	      *p_os << "]";
	    }

	    *p_os << "]";
	  }
	} /*print_json*/
      }; /*MatrixXdJsonPrinter*/

      template<typename EigenVectorType>
      void
      provide_eigen_vector_printer(PrintJson * p_pjson)
      {
	TypeDescr td = Reflect::require<EigenVectorType>();
	std::unique_ptr<JsonPrinter> pr(new EigenVectorJsonPrinter<EigenVectorType>(p_pjson));

	p_pjson->provide_printer(td, std::move(pr));
      } /*provide_eigen_vector_printer*/
    } /*namespace*/

    void
    EigenUtil::reflect_eigen()
    {
#ifdef NOT_YET
      Reflect::require<VectorXb>();
      Reflect::require<VectorXd>();
#endif
    } /*reflect_eigen*/

    void
    EigenUtil::provide_json_printers(PrintJson * p_pjson)
    {
      assert(p_pjson);
      
      provide_eigen_vector_printer<VectorXb>(p_pjson);
      provide_eigen_vector_printer<VectorXd>(p_pjson);

      {
	TypeDescr td = Reflect::require<MatrixXd>();
	std::unique_ptr<JsonPrinter> pr(new MatrixXdJsonPrinter(p_pjson));

	p_pjson->provide_printer(td, std::move(pr));
      }
    } /*provide_json_printers*/
  } /*namespace eigen*/
} /*namespace xo*/

/* end EigenUtil.cpp */
