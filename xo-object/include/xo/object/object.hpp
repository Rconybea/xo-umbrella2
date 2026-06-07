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
            ot_invalid  = static_cast<uint16_t>(-1),
            ot_sentinel = 0x00,
            ot_boolean  = 0x01,
            ot_char     = 0x02,
            ot_i32      = 0x03,
            ot_f32      = 0x04,

            /* 0x05, 0x06, 0x07 reserved */

            ot_i64      = 0x08,
            ot_f64      = 0x09,
            ot_zstring  = 0x0a,
            ot_symbol   = 0x0b,
            ot_cons     = 0x0c,

            /* 0x0d, 0x0e reserved */
            ot_rc_object = 0x10,   /* refcounted pointer to xo::reflect::Object */
        };

        class cons;
        class zstring;
        class symbol;

        /** @class object
         *
         *  @brief dynamically tyyped object; support for schematica
         **/
        class object {
        public:
            using Object = xo::reflect::Object;

            /** tags involving pointers use values 0x08 .. 0x10 **/
            static constexpr uint16_t c_tag_pointer_mask = 0xfff8;

            /** number of pointer bits stolen for type tag **/
            static constexpr int c_tag_bits = 16;
            /** number of pointer bits remaining after hi tag bits stolen **/
            static constexpr int c_ptr_bits = 64 - c_tag_bits;

            static constexpr std::uint64_t c_tag_mask = (0UL - 1) << c_ptr_bits;
            static constexpr std::uint64_t c_ptr_mask = (0UL - 1) >> c_tag_bits;

        public:
            object() = default;
            object(const object & x);

            otag tag() const noexcept { return (static_cast<otag>(value_ >> c_ptr_bits)); }
            std::uint64_t masked_value() const noexcept { return value_ & c_ptr_mask; }

            std::int32_t as_int32(std::int32_t sentinel = 0) const noexcept {
                if (tag() == otag::ot_i32) {
                    return masked_value();
                } else {
                    return sentinel;
                }
            }

            float as_float32(float sentinel = std::numeric_limits<float>::quiet_NaN()) const noexcept {
                if (tag() == otag::ot_f64) {
                    std::uint32_t bits = masked_value();

                    return * reinterpret_cast<float *>(&bits);
                } else {
                    return sentinel;
                }
            }

            std::int64_t as_int64(int64_t sentinel = 0) const noexcept {
                if (tag() == otag::ot_i64) {
                    return cast_int64();
                } else {
                    return sentinel;
                }
            }

            double as_float64(double sentinel = std::numeric_limits<double>::quiet_NaN()) const noexcept {
                if (tag() == otag::ot_f64) {
                    return * reinterpret_cast<double  *>(masked_value());
                } else {
                    return sentinel;
                }
            }

            zstring * as_zstring() const noexcept {
                if (tag() == otag::ot_zstring) {
                    return reinterpret_cast<zstring *>(masked_value());
                } else {
                    return nullptr;
                }
            }

            Object * as_object() const noexcept {
                if (tag() == otag::ot_rc_object) {
                    return reinterpret_cast<Object *>(masked_value());
                } else {
                    return nullptr;
                }
            }

        private:
            /** ctor.  only use least-significant c_ptr_bits (48) bits from value **/
            explicit object(otag tag, std::uint64_t value) {
                this->set_tag_value(tag, value);
            }

            /** undefined behavior if tag != ot_i64 **/
            std::int64_t cast_int64() const noexcept {
                return * reinterpret_cast<std::int64_t *>(masked_value());
            }

            /** undefined behavior if tag != ot_f64 **/
            double cast_double() const noexcept {
                return * reinterpret_cast<double *>(masked_value());
            }

            zstring * cast_zstring() const noexcept {
                return reinterpret_cast<zstring *>(masked_value());
            }

            symbol * cast_symbol() const noexcept {
                return reinterpret_cast<symbol *>(masked_value());
            }

            Object * cast_object() const noexcept {
                return reinterpret_cast<Object *>(masked_value());
            }

            /** only use bottom c_ptr_bits (48) from value **/
            void set_tag_value(otag tag, std::uint64_t value) {
                value_ = (static_cast<std::uint16_t>(tag) | (value & c_ptr_mask));
            }

        private:
            /**
             *  Rely on being able to steal the 16 most-significant
             *  bits from a 64-bit pointer
             *
             * @code
             *
             * - ot_i64, ot_f64, ot_zstring, ot_symbol, ot_cons, ot_rc_object:
             *
             *     <- 16 -> <---------- 48 ---------->
             *    +--------+--------------------------+
             *    |    tag |                     ptr  |
             *    +--------+--------------------------+
             *
             * - ot_f32:
             *
             *     <- 16 -> <- 16 -> <------ 32 ----->
             *    +--------+--------+-----------------+
             *    | ot_f32 | unused |  32-bit float   |
             *    +--------+--------+-----------------+
             *
             * - ot_boolean
             *
             *     <- 16 -> <-------- 47 ----------> 1
             *    +--------+------------------------+-+
             *    | ot_f32 |        unused          |b|
             *    +--------+------------------------+-+
             *
             * etc...
             *
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
