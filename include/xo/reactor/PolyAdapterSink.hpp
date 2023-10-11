/* file PolyAdapterSink.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "Sink.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
    namespace reactor {
        /* adapter between a source that delivers a particular event type T,
         * and a sink that accepts arbitrarily-typed events via .notify_ev_tp()
         * Use this to connect to a polymorphic sink.
         *
         * Require:
         * - .poly_sink.allow_polymorphic_source()
         *   (ofc.  otherwise no point in using PolyAdapterSink<T>)
         * - .poly_sink.allow_volatile_source()
         *   need this bc will be wrapping event with TaggedPtr,
         *   which doesn't manage event lifetime
         */
        template<typename T>
        class PolyAdapterSink : public reactor::Sink1<T> {
        public:
            using Reflect = reflect::Reflect;
            using TaggedPtr = reflect::TaggedPtr;

        public:
            /* named ctor idiom */
            static ref::rp<PolyAdapterSink> make(ref::rp<AbstractSink> poly_sink) {
                //xo::scope lscope("PolyAdapterSink::make");

                ref::rp<PolyAdapterSink> retval(new PolyAdapterSink(poly_sink));

                //lscope.log("adapter", (void*)retval.get());

                return retval;
            } /*make*/

            // ----- Inherited from Sink1<T> -----

            virtual void notify_ev(T const & ev) override {
                //xo::scope lscope("PolyAdapterSink::notify_ev");
                //lscope.log(xo::xtag("ev", ev));

                TaggedPtr ev_tp = Reflect::make_tp(const_cast<T *>(&ev));

                this->notify_ev_tp(ev_tp);
            } /*notify_ev*/

            // ----- Inherited from AbstractSink -----

            virtual bool allow_volatile_source() const override { return true; }
            virtual uint32_t n_in_ev() const override  { return this->poly_sink_->n_in_ev(); }
            /* note: ok to do this,  however if expecting to use this entry point,
             *       maybe don't need to interpose PolyAdapterSink<T> ahead of .poly_sink
             */
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override {
                //xo::scope lscope("PolyAdapterSink::notify_ev_tp");

                return this->poly_sink_->notify_ev_tp(ev_tp);
            }

            // ----- Inherited from AbstractEventProcessor -----

            virtual std::string const & name() const override { return this->poly_sink_->name(); }
            virtual void set_name(std::string const & x) override { this->poly_sink_->set_name(x); }
            virtual void visit_direct_consumers(std::function<void (ref::brw<AbstractEventProcessor> ep)> const & fn) override {
                this->poly_sink_->visit_direct_consumers(fn);
            }
            virtual void display(std::ostream & os) const override {
                using xo::xtag;
                os << "<PolyAdapterSink"
                   << xtag("addr", (void*)this)
                   << xtag("T", reflect::type_name<T>())
                   << xtag("poly", this->poly_sink_)
                   << ">";
            } /*display*/

        private:
            PolyAdapterSink(ref::rp<AbstractSink> poly_sink) : poly_sink_{std::move(poly_sink)} {}

        private:
            /* mandate: .poly_sink.allow_polymorphic_source() is true */
            ref::rp<AbstractSink> poly_sink_;
        }; /*PolyAdapterSink*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end PolyAdapterSink.hpp */
