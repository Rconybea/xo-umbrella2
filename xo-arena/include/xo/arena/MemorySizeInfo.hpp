/** @file MemorySizeInfo.hpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include <string_view>
#include <cstddef>

namespace xo {
    namespace mm {

        struct MemorySizeInfo {
            using size_type = std::size_t;

            MemorySizeInfo(std::string_view name, std::size_t a, std::size_t c, std::size_t r)
              : resource_name_{name}, allocated_{a}, committed_{c}, reserved_{r}
            {}

            static MemorySizeInfo sentinel() { return MemorySizeInfo("", 0, 0, 0); }

            /** resource name **/
            std::string_view resource_name_;
            /** memory in-use **/
            std::size_t allocated_ = 0;
            /** memory committed (backed by physical memory) **/
            std::size_t committed_ = 0;
            /** memory reserved:
             *  virtual memory addresses range obtained, whether or not committed
             **/
            std::size_t reserved_ = 0;
        };

        /** function that visits MemorySizeInfo for a collection of @p n memory pools.
         *  Each pool reported with index @p i in [0, n), with associated
         *  size record @p info.
         **/
        using MemorySizeVisitor = std::function<void (const MemorySizeInfo & info)>;
    }
}

/* end MemorySizeInfo.hpp */
