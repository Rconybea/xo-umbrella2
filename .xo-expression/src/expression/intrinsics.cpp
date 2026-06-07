/* @file intrinsics.cpp */

#include "intrinsics.hpp"

/* FIXME: don't know how to mangle symbols yet,
 * so putting functions invoked from jit into global namespace
 */
extern "C"
int32_t
mul_i32(int32_t x, int32_t y) {
    return x * y;
}

/* end intrinsics.cpp */
