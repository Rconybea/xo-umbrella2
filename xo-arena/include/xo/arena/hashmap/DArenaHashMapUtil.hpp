/** @file DArenaHashMapUtil.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

namespace xo {
    namespace mm {
        /** @class DArenaHashMapUtil
         *
         *  @pre
         *
         *  control
         *
         *   <----------------- control_size(n_slot) ---------------->
         *   <-stub-> <----------- n_slot ----------> <group> <-stub->
         *  +--------+-------------------------------+-------+--------+
         *  | 0xF0   | empty / data / tombstone      | wrap  | 0xF0   |
         *  +--------+-------------------------------+-------+--------+
         *             ^                           ^
         *             |            ...            | control_[stub+i] <--> slots_[i]
         *  slots      v                           v
         *           +-------------------------------+
         *           | {k,v} pairs                   |
         *           +-------------------------------+
         *            <--- n_slot key-value pairs -->
         *
         *  sizes:
         *  - stub    before+after bookends.  c_control_stub bytes (16)
         *  - group   c_group_size. power of 2 (16 bytes)
         *  - n_slot  hash table slots. power of 2 multiple of c_group_size.
         *
         *  control bytes:
         *  - 0b1xxxxxxx  sentinel bitmask
         *  - 0xf0        sentinel for before/after stubs (iterator bookends)
         *  - 0xff        sentinel for empty slot.
         *  - 0xfe        sentinel for tombstone
         *  - 0b0xxxxxxx  high bit clear; remainder hold low 7 bits of hash
         *  - wrap        duplicate first c_group_size bytes (after front stub)
         *                for SIMD convenience
         *
         *  @endpre
         **/
        struct DArenaHashMapUtil {
            using size_type = std::size_t;
            using control_type = std::uint8_t;

            /** control: mask for sentinel states **/
            static constexpr uint8_t c_sentinel_mask = 0xF0;
            /** control: sentinel for empty slot **/
            static constexpr uint8_t c_empty_slot = 0xFF;
            /** control: tombstone for deleted slot **/
            static constexpr uint8_t c_tombstone = 0xFE;
            /** control: bookends around control array,
             *  for iterator edge support
             **/
            static constexpr uint8_t c_iterator_bookend = 0xF0;

            /** group size **/
            static constexpr size_type c_group_size = 16;

            /** max load factor **/
            static constexpr float c_max_load_factor = 0.875;

            /** Iterator sentinel at begin/end of control array.
             *  Load-bearing for bidirectional iterator implementation
             **/
            static constexpr size_type c_control_stub = c_group_size; //c_group_size;

            /** control: true for sentinel values **/
            static constexpr bool is_sentinel(control_type ctrl) {
                return c_sentinel_mask == (ctrl & c_sentinel_mask);
            }

            /** control; true for non-sentinel values **/
            static constexpr bool is_data(control_type ctrl) {
                return 0 == (ctrl & c_sentinel_mask);
            }

            /** control: compute size of control array for swiss hash map with @p n_slot cells **/
            static constexpr size_type control_size(size_type n_slot) {
                return n_slot + c_group_size + 2 * c_control_stub;
            }

            /** find smallest multiple k : k * c_group_size >= n **/
            static size_type lub_group_mult(size_t n) {
                return (n + c_group_size - 1) / c_group_size;
            }

            /** find smallest x such that 2^x >= n. Return {x, 2^x} **/
            static std::pair<size_type, size_type> lub_exp2(size_t n) {
                size_type ngx = 0;
                size_type ng = 1;

                while (ng < n) {
                    ++ngx;
                    ng *= 2;
                }

                return std::make_pair(ngx, ng);;
            }
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end DArenaHashMapUtil.hpp */
