/** @file DTypedSink.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <xo/reflect/TypeDescr.hpp>

namespace xo::reactor {
    /** @brief AEventSink implementation for events of type T, with T known at compile time
     *
     *  Inherit to complete implementation.
     *n
     *  @tparam T : event type
     *  @tparam Derived : concrete implementation
     *  @tparam Fn : invoke this function to consume an event
     **/
    template <typename T, typename Derived, typename Fn>
    class DTypedSink {
    public:
        using Reflect = xo::reflect::Reflect;
        using TypeDescr = xo::reflect::TypeDescr;

    public:
        /** @defgroup reactor-eventsink-facet **/
        ///@{

        /** Typed sink only accepts events of type T.
         *
         *  Can interpose a PolyAdapterSink<T> upstream of a DTypedSink<T>
         *  to filter at runtime
         **/
        bool allow_polymorphic_source() const noexcept { return false; }

        /** runtime description of T **/
        TypeDescr sink_ev_type() const noexcept { return Reflect::require<T>(); }

        /** lifetime counter for incoming events **/
        uint32_t n_in_ev() const noexcept { return n_in_ev_; }

        /** consume variant event. Typed sink will accept only events of type T **/
        void notify_ev_tp(const TaggedPtr & ev_tp) {
            T * p_ev = ev_tp.recover_native<T>();

            if (p_ev) {
                auto * self = reinterpret_cast<Derived*>(this);

                self->notify_ev(*p_ev);
            } else {
                // TODO: explicit allocator

                throw std::runtime_error(tostr("DTypedSink::notify_ev_tp",
                                               ": unable to convert ev_tp to T",
                                               xtag("ev_tp.type", ev_tp.td()->canonical_name()),
                                               xtag("T", reflect::type_name<T>())));
            }
        }

        ///@}

    protected:
        /** counts lifetime number of incoming events **/
        uint32_t n_in_ev_ = 0;
    };

    template <typename T, typename ConsumeFn>
    class DSinkToFunction : public DTypedSink<T, DSinkToFunction, ConsumeFn>
    {
        bool allow_volatile_source() const noexcept { return false; }

        void notify_ev(const T & ev) {
            ++(this->n_in_ev_):
            // TODO: may need to expand signature?
            consume_fn_(ev);
        }

        void pretty(PpSink & sink) const {
            const auto name = this->name();
            const auto n_ev = this->n_in_ev();

            sink.pretty_struct("SinkToFunction",
                               field("name", name),
                               field("n_in_ev", n_ev));
        }

    private:
        /** event consumer **/
        ConsumeFn consume_fn_;
    };
}

/* end DTypedSink.hpp */
