/* @file pydistribution.cpp */

#include "pydistribution.hpp"
#include "xo/distribution/Normal.hpp"
#include "xo/distribution/ExplicitDist.hpp"
//#include "xo/reflect/SelfTagging.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace xo {
    using xo::distribution::Normal;
    using xo::distribution::Distribution;
    using xo::distribution::ExplicitDist;

    namespace sim {
        namespace py = pybind11;

        PYBIND11_MODULE(PYDISTRIBUTION_MODULE_NAME(), m) {
            m.doc() = "pybind11 distribution plugin"; // optional module docstring

            m.def("normalcdf",
                  &Normal::cdf_impl,
                  "cumulative normal distribution",
                  py::arg("x"));

            py::class_<Distribution<double>,
                       rp<Distribution<double>>>(m, "Distribution")
                .def("cdf", &Distribution<double>::cdf,
                     "return cumulative distribution function at x",
                     py::arg("x"));

            py::class_<ExplicitDist<double>,
                       Distribution<double>,
                       rp<ExplicitDist<double>>>(m, "ExplicitDist")
                .def_static("make", &ExplicitDist<double>::make,
                            "create instance",
                            py::arg("bucket_dx"), py::arg("ref_value"))
                .def_static("make_n", &ExplicitDist<double>::make_n,
                            "create instance with n buckets",
                            py::arg("n"), py::arg("bucket_dx"), py::arg("ref_value"))
                .def("n_bucket", &ExplicitDist<double>::n_bucket,
                     "return number of explicitly-represented buckets in distribution")
                .def("lo", &ExplicitDist<double>::lo,
                     "return least upper bound x: cdf(x)=0")
                .def("hi", &ExplicitDist<double>::hi,
                     "return greatest lower bound x: cdf(x)=1")
                .def("density", &ExplicitDist<double>::density,
                     "return probability density at x",
                     py::arg("x"))
                .def("density_v", &ExplicitDist<double>::density_v,
                     "return probability density vector for all explicit buckets."
                     " each member is pair {lh bucket edge, density}")
                .def("signed_bucket_index", &ExplicitDist<double>::signed_bucket_index,
                     "signed index to probability bucket.  ref_value -> 0",
                     py::arg("x"))
                .def("scale_bucket", &ExplicitDist<double>::scale_bucket,
                     "scale probability weight in bucket containing x by k",
                     py::arg("x"), py::arg("k"))
                .def("scale_by_normal_cdf", &ExplicitDist<double>::scale_by_normal_cdf,
                     "scale by normal cumulative distribution N(sign.(x-mean)/sigma)."
                     " expect sign in {+1, -1}",
                     py::arg("sign"), py::arg("mean"), py::arg("sigma"))
                .def("renormalize", &ExplicitDist<double>::renormalize,
                     "renormalize to ensure sum of weights=1")
                .def("check_renormalize", &ExplicitDist<double>::check_renormalize,
                     "renormalize if needed,  otherwise do nothing")
                .def("__repr__", &ExplicitDist<double>::display_string);
        }
    } /*namespace sim*/
} /*namespace xo*/

/* end pydistribution.cpp */
