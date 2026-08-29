/** @file DTypedEvent.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <xo/reflect/StructReflector.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <xo/timeutil/timeutil.hpp>

namespace xo::process {
    /** @brief A typical representation for an event.
     *
     *  A DTypedEvent atttaches a timestamp to an otherwise untyped event.
     **/
    template <typename T>
    class DTypedEvent {
    public:
        using PpSink = xo::pp::PpSink;
        using utc_nanos = xo::time::utc_nanos;

    public:
        DTypedEvent() = default;
        DTypedEvent(utc_nanos tm, const T & x) : tm_{tm}, value_{x} {}

        /** reflect DTypedEvent object representation **/
        static void reflect_self();

        utc_nanos tm() const { return tm_; }
        const T & value() const { return value_; }

        void pretty(PpSink & sink) const;

    protected:
        /** event timestamp **/
        utc_nanos tm_;
        /** event value **/
        T value_;
    };

    template <typename T>
    void DTypedEvent<T>::reflect_self()
    {
        using xo::reflect::StructReflector;

        StructReflector<DTypedEvent<T>> sr;

        if (sr.is_incomplete()) {
            REFLECT_MEMBER(sr, tm);
            REFLECT_MEMBER(sr, value);
        }
    }

    template <typename T>
    void DTypedEvent<T>::pretty(PpSink & sink) const
    {
        using xo::pp::field;

        sink.pretty_struct("TypedEvent",
                           field("tm", tm_),
                           field("value", value_));
    }
} /*xo::process*/

namespace xo::pp {
    template <typename T>
    struct Prettifier<xo::process::DTypedEvent<T>> {
        static void print(PpSink & sink, const xo::process::DTypedEvent<T> & x) {
            x.pretty(sink);
        }
    };
}

/* end DTypedEvent.hpp */
