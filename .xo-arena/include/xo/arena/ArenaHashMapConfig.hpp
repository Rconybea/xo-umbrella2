/** @file ArenaHashMapConfig.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include <string>
#include <cstdint>

namespace xo {
    namespace map {
        /** @class ArenaHashMapConfig
         *
         *  @brief configuration for a @ref DArenaHashMap instance
         **/
        struct ArenaHashMapConfig {
            /** @defgroup map-arenahashmapconfig-ctors **/
            ///@{

            ArenaHashMapConfig with_name(std::string name) const {
                ArenaHashMapConfig copy(*this);
                copy.name_ = name;
                return copy;
            }

            ArenaHashMapConfig with_hint_max_capacity(std::size_t z) const {
                ArenaHashMapConfig copy(*this);
                copy.hint_max_capacity_ = z;
                return copy;
            }

            ArenaHashMapConfig with_debug_flag(bool x) const {
                ArenaHashMapConfig copy(*this);
                copy.debug_flag_ = x;
                return copy;
            }

            ///@}
            /** @defgroup mm-arenahashmapconfig-instance-vars ArenaHashMapConfig members **/
            ///@{

            /** optional name, for diagnostics **/
            std::string name_;
            /** desired hard max hashmap size -> reserved virtual memory
             *  hint: actual max may be larger, because of power-of-2 considerations.
             **/
            std::size_t hint_max_capacity_ = 0;
            /** true to enable debug logging **/
            bool debug_flag_ = false;

            ///@}
        };

    } /*namespace map*/
} /*namespace xo*/

/* end ArenaHashMapConfig.hpp */
