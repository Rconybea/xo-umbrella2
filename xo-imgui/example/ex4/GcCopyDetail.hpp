/* GcCopyDetail.hpp */

#pragma once

#include "xo/alloc/generation.hpp"
#include <cstdint>

/** details of a single copy event performed by GC **/
struct GcCopyDetail {
    using generation = xo::gc::generation;

public:
    GcCopyDetail(std::size_t z,
                 generation src, std::size_t src_offset, std::size_t src_space_z,
                 generation dest, std::size_t dest_offset, std::size_t dest_z)
        : z_{z},
          src_gen_{src}, src_offset_{src_offset}, src_space_z_{src_space_z},
          dest_gen_{dest}, dest_offset_{dest_offset}, dest_z_{dest_z}
        {}

public:
    /** object size in bytes **/
    std::size_t z_ = 0;
    /** source location **/
    generation src_gen_;
    /** offset from start of allocator **/
    std::size_t src_offset_ = 0;
    /** size of source space.  could store this separately **/
    std::size_t src_space_z_ = 0;

    /** destination location **/
    generation dest_gen_;
    /** offset from start of allocator **/
    std::size_t dest_offset_ = 0;
    /** size of destination space. (could store this separately). **/
    std::size_t dest_z_ = 0;
};

/* end GcCopyDetail.hpp */
