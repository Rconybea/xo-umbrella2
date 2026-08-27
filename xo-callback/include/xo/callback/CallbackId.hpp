/** @file CallbackId.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <cstdint>

namespace xo::fn {
    /* identifies a particular callback in a CallbackSet (see below).
     * an unique id is created:
     *   CallbackSetImpl cbset = ...;
     *   CallbackId cb_id = cbset.add_callback(..);
     *
     * can use id to remove callback later:
     *   cbset.remove_callback(cb_id);
     *
     * Tag so xo-callback can be header-only
     */
    template <typename Tag>
    class CallbackIdImpl {
    public:
        CallbackIdImpl() = default;
        explicit CallbackIdImpl(uint32_t id) : id_{id} {}

        /* generate a globally-unique id (not threadsafe) */
        static CallbackIdImpl generate() {
            static CallbackIdImpl s_last_id;

            s_last_id = CallbackIdImpl(s_last_id.id() + 1);

            return s_last_id;
        } /*generate*/

        uint32_t id() const { return id_; }

    private:
        uint32_t id_ = 0;
    }; /*CallbackIdImpl*/

    template <typename Tag>
    inline bool operator==(CallbackIdImpl<Tag> lhs, CallbackIdImpl<Tag> rhs) { return lhs.id() == rhs.id(); }
    template <typename Tag>
    inline bool operator!=(CallbackIdImpl<Tag> lhs, CallbackIdImpl<Tag> rhs) { return lhs.id() != rhs.id(); }

    using CallbackId = CallbackIdImpl<class CallbackId_tag>;

}

/* end CallbackId.hpp */
