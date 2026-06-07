/** @file object.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/reflect/Object.hpp"
#include <cstdint>

namespace xo {
    namespace obj {
        enum class otag : std::uint16_t {
            vt_invalid  = -1,
            vt_sentinel = 0x00,
            vt_boolean  = 0x01,
            vt_char     = 0x02,
            vt_i32      = 0x03,
            vt_f32      = 0x04,
            vt_i64      = 0x08,
            vt_f64      = 0x09,
            vt_list     = 0x0a,
            vt_object   = 0x10,
        };

        class cons;

        /** @class object
         *
         *  @brief dynamically tyyped object; support for schematica
         **/
        class object {
        public:
            using Object = xo::reflect::Object;

            /** number of pointer bits stolen for type tag **/
            static constexpr int c_tag_bits = 16;
            /** number of pointer bits remaining after hi tag bits stolen **/
            static constexpr int c_ptr_bits = 64 - c_tag_bits;

            static constexpr std::uint64_t c_ptr_mask = (0UL - 1) >> c_tag_bits;

        public:
            object() = default;

            otag tag() const noexcept { return reinterpret_cast<vtag>(value_ >> c_ptr_bits); }
            std::uint64_t masked_value() const noexcept { return value_ & c_ptr_mask; }

            std::int32_t as_int32(std::int32_t sentinel = 0) const noexcept {
                if (tag() == vtag::vt_i32) {
                    return masked_value();
                } else {
                    return sentinel;
                }
            }

            float as_float32(float sentinel = std::numeric_limits<float>::quiet_NaN()) const noexcept {
                if (tag() == vtag::vt_f64) {
                    return masked_value();
                } else {
                    return sentinel;
                }
            }

            std::int64_t as_int64(int64_t sentinel = 0) const noexcept {
                if (tag() == vtag::vt_i64) {
                    return * reinterpret_cast<std::int64_t *>(masked_value());
                } else {
                    return sentinel;
                }
            }

            double as_float64(double sentinel = std::numeric_limits<double>::quiet_NaN()) const noexcept {
                if (tag() == vtag::vt_f64) {
                    return * reinterpret_cast<double  *>(masked_value());
                } else {
                    return sentinel;
                }
            }

            Object * as_object() const noexcept {
                if (tag() == vtag::vt_object) {
                    return reinterpret_cast<Object *>(masked_value());
                } else {
                    return nullptr;
                }
            }

        private:
            /**
             *  Rely on being able to steal the 16 most-significant
             *  bits from a 64-bit pointer
             *
             * @code
             *     <- 16 -> <---------- 48 ---------->
             *    +--------+--------------------------+
             *    |    tag |                          |
             *    +--------+--------------------------+
             * @endcode
             *
             * tag values given by @ref otag
             *
             * 0x00 -> sentinel. sentinel value.  not accessible from schematica
             * 0x01 -> boolean.  truth value in least-significant bit
             * 0x02 -> char.     ascii character in least-significant 8 bits
             * 0x03 -> int.      integer in least-significant 32 bits
             * 0x04 -> float.    32-bit float in least-significant 32 bitsg
             *
             * 0x05..0x07 reserved
             *
             * 0x08 -> long.     ptr refers to 64-bit integer
             * 0x09 -> double.   ptr refers to 64-bit floating-point value
             * 0x0a -> zstring.  ptr refers to length-prefixed null-terminated cstring.
             * 0x0b -> symbol.   ptr refers to interned (unique'd) symbol
             * 0x0c -> cons.     ptr refers to 128-bit cons-cell (or is nullptr, representing nil)
             *
             * 0x0d..0x0f reserved
             *
             * 0x10 -> Object.   ptr to refcounted xo::obj::Object
             *                   see xo-reflect/ Object.hpp
             **/
            std::uint64_t value_ = 0;
        };
    } /*namespace obj*/
} /*namespace xo*/


/** end object.hpp **/
