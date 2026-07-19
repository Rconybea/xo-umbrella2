/** @file PrettyPrinter.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpSink.hpp"
#include "PpState.hpp"

namespace xo {
    namespace print {
        /**
         *  Use:
         *  @code
         *    PpConfig ppc;
         *    PrettyPrinter pp(ppc);
         *
         *  @endcode
         **/
        class PrettyPrinter : public PpSink {
        public:
            PrettyPrinter(const PpConfig & cfg);

            // inherited from PpSink

            virtual void put(std::string_view x) override final;
            virtual void begin() override final;
            virtual void split() override final;
            virtual void end() override final;

        private:
            /** Pretty printer state (+ driver) **/
            PpState pps_;
            /** Buffer for pretty-printed output **/
            LogBuffer logbuf_;
        };
    } /*namespace print*/
} /*namespace xo*/

/* end PrettyPrinter.hpp */
