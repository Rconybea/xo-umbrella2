/* @file pyreflect.hpp
 *
 * automatically generated from src/xo_pyreflect/pyreflect.hpp.in
 * see src/xo_pyreflect/CMakeLists.txt
 */

/* python requires module name = library name
 * example:
 *   PYBIND11_MODULE(PYREFLECT_MODULE_NAME(), m) { ... }
 */
#define PYREFLECT_MODULE_NAME() pyreflect

/* example:
 *   py::module_::import(PYREFLECT_MODULE_NAME_STR)
 */
#define PYREFLECT_MODULE_NAME_STR "pyreflect"

/* example:
 *   PYREFLECT_IMPORT_MODULE()
 * replaces
 *   py::module_::import("pyreflect")
 */
#define PYREFLECT_IMPORT_MODULE() py::module_::import("pyreflect")

/* end pyreflect.hpp */
