/* @file Sink.hpp */

#pragma once

#include "AbstractSink.hpp"
#include "AbstractSource.hpp"
#include "PolyAdapterSink.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << xo::pp::xtag(..) */
#include <xo/timeutil/timeutil.hpp>
#include <xo/cxxutil/demangle.hpp>
#include <typeinfo>
#include <xo/ppsink/pretty_ostream.hpp>   /* pp_to_stream */
#include <xo/ppsink/pretty_pair.hpp>      /* Prettifier<std::pair<T,U>> */

/* NB xo::pp names are QUALIFIED throughout this header, not brought in by
 * using-declarations.  Two reasons:
 *   - a using-decl at namespace scope in a public header leaks into every
 *     consumer's scope;
 *   - a function-local one is not enough either: ADL adds legacy xo::xtag
 *     whenever an argument type lives in namespace xo, and merges it into
 *     the candidate set.  Only a qualified call avoids that.
 */

namespace xo {
    namespace reactor {
        /* Sink for events of type T
         *
         * inheritance:
         *   ref::Refcount
         *     ^
         *     isa
         *     |
         *   reactor::AbstractEventProcessor
         *     ^
         *     isa
         *     |
         *   reactor::AbstractSink
         *     ^
         *     isa
         *     |
         *   reactor::Sink1<T>
         */
        template<typename T>
        class Sink1 : public AbstractSink {
        public:
            using Reflect = reflect::Reflect;
            using TypeDescr = reflect::TypeDescr;

        public:
            /* convenience:  convert abstract sink to Sink1<T>*,
             * or throw
             */
            static rp<Sink1<T>> require_native(std::string_view caller,
                                               rp<AbstractSink> const & sink)
                {

                    /* 1. if sink expects events of type T,
                     *    make direct connection
                     */
                    Sink1<T> * native_sink = nullptr;

                    native_sink = dynamic_cast<Sink1<T> *>(sink.get());

                    if (native_sink)
                        return native_sink;

                    /* 2. if sink is polymorphic,
                     *    make type-erasing adapter
                     */

                    if (sink->allow_polymorphic_source()) {
#ifdef DEBUG_NOT_USING
                        xo::pp::scope lscope("Sink1<T>::require_native: create PolyAdapterSink");
                        lscope.log(xo::pp::xtag("caller", caller));
#endif

                        return PolyAdapterSink<T>::make(sink);
                    }

                    if (!native_sink) {
#ifdef DEBUG_EVENT_TYPEINFO
                        std::type_info const * sink_parent_typeinfo
                            = sink->parent_typeinfo();
#endif

                        std::size_t src_hashcode = typeid(T).hash_code();

                        throw std::runtime_error(xo::pp::tostr("Sink1<T>::require_native"
                                                       ": wanted to sink S,  but sink expects T",
                                                       xo::pp::xtag("caller", caller),
                                                       xo::pp::xtag("T", sink->sink_ev_type()->canonical_name()),
                                                       xo::pp::xtag("S", reflect::type_name<T>()),
                                                       xo::pp::xtag("required_hashcode", typeid(Sink1<T>).hash_code()),
                                                       xo::pp::xtag("required_name", typeid(Sink1<T>).name()),
                                                       xo::pp::xtag("src_hashcode", src_hashcode),
                                                       xo::pp::xtag("sink_hashcode", sink->sink_ev_type()->native_typeinfo()->hash_code())
#ifdef DEBUG_EVENT_TYPEINFO
                                                       , xo::pp::xtag("sink_hashcode", sink->item_typeinfo()->hash_code())
                                                       , xo::pp::xtag("sink_parent_hashcode", sink_parent_typeinfo->hash_code())
                                                       , xo::pp::xtag("sink_parent_name", sink_parent_typeinfo->name())
                                                       , xo::pp::xtag("sink.type", sink->self_typename())
                                                       , xo::pp::xtag("sink.parent_type", sink->parent_typename())
#endif
                                                     ));
                    }

                    return native_sink;
                } /*require_native*/

            virtual TypeDescr sink_ev_type() const override { return reflect::Reflect::require<T>(); }
            /* accept incoming event */
            virtual void notify_ev(T const & ev) = 0;

            /* invoke these when this sink added to, or removed from, a source */
            virtual void notify_add_callback() {}
            virtual void notify_remove_callback() {}

            // ----- inherited from AbstractSink -----

            /* Sink1<T> only allows source providing T */
            virtual bool allow_polymorphic_source() const override { return false; }

            virtual void attach_source(rp<AbstractSource> const & src) override {
                src->attach_sink(this);
            } /*attach_source*/

            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override {

                T * p_ev = ev_tp.recover_native<T>();

                if (p_ev) {
                    this->notify_ev(*p_ev);
                } else {
                    throw std::runtime_error(xo::pp::tostr("Sink1<T>::notify_ev_tp"
                                                   ": unable to convert ev_tp to T",
                                                   xo::pp::xtag("ev_tp.type", ev_tp.td()->canonical_name()),
                                                   xo::pp::xtag("T", reflect::type_name<T>())));
                }
            } /*notify_ev_tp*/
        }; /*Sink1*/

        /* a sink with no further downstream processors */
        template<typename T>
        class SinkEndpoint : public Sink1<T> {
        public:
            // ----- Inherited from AbstractEventProcessor -----

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { name_ = x; }

            virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor>)> const &) override {
                /* *this is not an event source */
            } /*visit_direct_consumers*/

        private:
            /* reporting name for this sink */
            std::string name_;
        }; /*SinkEndpoint*/

        template<typename T, typename Fn>
        class SinkToFunction : public SinkEndpoint<T> {
        public:
            SinkToFunction(Fn fn) : fn_{std::move(fn)} {}

            /* NOTE: conservative choice here,  could templatize on this */
            virtual bool allow_volatile_source() const override { return false; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void notify_ev(T const & ev) override {
                ++(this->n_in_ev_);
                fn_(ev);
            } /*notify_ev*/

            virtual void display(std::ostream & os) const override {

                os << "<SinkToFunction"
                   << xo::pp::xtag("name", this->name())
                   << xo::pp::xtag("n_in_ev", this->n_in_ev())
                   << ">";
            } /*display*/

        private:
            Fn fn_;
            /* counts lifetime #of incoming events (see .notify_ev()) */
            uint32_t n_in_ev_ = 0;
        }; /*SinkToFunction*/

        /* sink that prints to console */
        template<typename T>
        class SinkToConsole : public SinkEndpoint<T> {
        public:
            SinkToConsole() {}

            virtual bool allow_volatile_source() const override { return true; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void notify_ev(T const & ev) override {
                //using logutil::operator<<;

                ++(this->n_in_ev_);

                /* pp_to_stream, not `std::cout << ev`: routes through
                 * Prettifier<T> when there is one (e.g. std::pair, which
                 * legacy covered with an operator<< declared -- illegally --
                 * in namespace std), and falls back to operator<< otherwise.
                 */
                xo::pp::pp_to_stream(std::cout, ev);
                std::cout << std::endl;
            } /*notify_ev*/

            virtual void display(std::ostream & os) const override {

                os << "<SinkToConsole"
                   << xo::pp::xtag("name", this->name())
                   << xo::pp::xtag("n_in_ev", this->n_in_ev())
                   << ">";
            } /*display*/

        private:
            /* reporting name for this sink */
            std::string name_;
            /* counts lifetime #of incoming events (see .notify_ev()) */
            uint32_t n_in_ev_ = 0;
        }; /*SinkToConsole*/

#ifdef NOT_USING
        class TemporaryTest {
        public:
            static rp<SinkToConsole<std::pair<xo::time::utc_nanos, double>>> realization_printer();
        }; /*TemporaryTest*/
#endif
    } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.hpp */
