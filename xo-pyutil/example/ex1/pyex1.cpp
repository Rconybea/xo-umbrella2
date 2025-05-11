/* @file pyex1.cpp */

#include "pyutilexample.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <cmath>

namespace xo {

    PYBIND11_MODULE(XO_PYUTILEXAMPLE_MODULE_NAME(), m) {
        m.def("sqrt", [](double x) { return ::sqrt(x); });
    }
}

/* end pyex1.cpp */
