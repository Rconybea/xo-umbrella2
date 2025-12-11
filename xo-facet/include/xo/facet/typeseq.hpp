/** @file typeseq.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <cstdint>

namespace xo {
    namespace facet {
        /**
         * Tag here so we can preserve header-only implementation
         * and still have static variable
         */
        template<typename Tag = class typeseq_tag>
        struct typeseq_impl {
            /** Can't have this be constexpr.
             *  We need ids in shared libraries to be generated
             *  at load time to avoid false positives
             *
             *  Return unique id number for each type.
             *  Numbers are sequentially allocated, so can use
             *  as vector indices
             *
             *  Conversely note that built-in typeinfo may
             *  return false negatives across library boundaries
             *  when using clang.
             **/
            template <typename T>
            static int32_t id() {
                static bool armed = true;
                static int32_t id = 0;

                if (armed) {
                    armed = false;
                    id = ++s_next_id;
                }

                return id;
            }

            static int32_t s_next_id;
        };

        template <typename Tag>
        int32_t typeseq_impl<Tag>::s_next_id = 0;

        using typeseq = typeseq_impl<>;
    }
} /*namespace xo*/

/* end typeseq.hpp */
