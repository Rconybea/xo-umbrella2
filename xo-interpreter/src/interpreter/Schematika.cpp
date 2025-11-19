/** @file Schematika.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "Schematika.hpp"
#include "VirtualSchematikaMachine.hpp"
#include "xo/reader/reader.hpp"
#include <replxx.hxx>
#include <ostream>

namespace xo {
    using xo::print::ppconfig;
    using xo::print::ppstate_standalone;
    using replxx::Replxx;
    using namespace std;

    namespace scm {

        class Schematika::Impl {
        public:
            Impl(const Config & config) : config_{config} {}

            /** borrow calling thread to run interactive read-eval-print loop;
             *  input from stdin, output to stdout.
             **/
            void interactive_repl();

            void welcome(std::ostream & os);

            bool replxx_getline(bool interactive,
                                std::size_t parser_stack_size,
                                replxx::Replxx & rx,
                                std::string & input);

        private:
            /** configuration **/
            Config config_;
            /** schematika interpreter **/
            VirtualSchematikaMachine vsm_;
        };

        void
        Schematika::Impl::welcome(std::ostream & os)
        {
            using namespace std;

            os << "read-eval-print loop for schematika expressions" << endl;
            os << "  ctrl-a/ctrl-e   beginning/end of line" << endl;
            os << "  ctrl-u          delete entire line" << endl;
            os << "  ctrl-k          delete to end of line" << endl;
            os << "  meta-<bs>       backward delete word" << endl;
            os << "  <up>|meta-p     previous command from history" << endl;
            os << "  <down>|meta-n   next command from history" << endl;
            os << "  <pgup>/<pgdown> page through history faster" << endl;
            os << "  ctrl-s/ctrl-r   forward/backward history search" << endl;
            os << endl;
        }

        // similar helper in exprreplxx.cpp
        //
        bool
        Schematika::Impl::replxx_getline(bool interactive,
                                         std::size_t parser_stack_size,
                                         replxx::Replxx & rx,
                                         std::string & input)
        {
            using namespace std;

            char const * prompt = "";

            if (interactive) {
                if (parser_stack_size <= 1)
                    prompt = "> ";
                else
                    prompt = ". ";
            }

            /* input_cstr: next line of input from replxx library */
            const char * input_cstr = rx.input(prompt);

            bool retval = (input_cstr != nullptr);

            if (retval) {
                /* got new input */
                input = input_cstr;
            }

            rx.history_add(input);

            input.push_back('\n');

            return retval;
        }

        void
        Schematika::Impl::interactive_repl()
        {
            using span_type = xo::scm::span<const char>;

            bool interactive = isatty(STDIN_FILENO);

            Replxx rx;
            rx.set_max_history_size(config_.history_size);
            rx.history_load(config_.history_file);
            //    rx.bind_key_internal(Replxx::KEY::control('p'), "history_previous");
            //    rx.bind_key_internal(Replxx::KEY::control('n'), "history_next");

            reader rdr(config_.debug_flag);
            rdr.begin_interactive_session();

            string input_str;

            bool eof = false;

            span_type input;
            std::size_t parser_stack_size = 0;

            if (config_.welcome_flag_)
                welcome(cerr);

            while (replxx_getline(interactive, parser_stack_size, rx, input_str)) {
                input  = span_type::from_string(input_str);

                while (!input.empty()) {
                    /**
                     *  Three cases here:
                     *  1. available input is invalid (does not conform to schematika syntax).
                     *     1a. expr=nullptr
                     *     1b. consumed reads all available input
                     *     1c. psz=0
                     *     1d. error.is_error(); details including exact location where parsing failed.
                     *     1e. parser reset to top level.
                     *  2. available input represents prefix of a possibly-valid expression
                     *     2a. expr=nullptr;
                     *     2b. consumed reads all available input
                     *     2c. psz reflects nesting level after reading available input.
                     *     2d. error.is_not_an_error()
                     *  3. available input completes at least one expression
                     *     3a. expr contains first completed top-level expression
                     *     3b. consumed reports portion of input up to end of expr
                     *     3c. psz=0
                     *     3d. error.is_not_an_error()
                     *
                     *  expr     :: rp<Expression>  if non-null: the next expression from input
                     *  consumed :: span<char>      extent of input read up to next Expression
                     *  psz      :: size_t          parser stack size
                     *  error    :: reader_error    error details on parsing failure
                     **/
                    auto [expr, consumed, psz, error] = rdr.read_expr(input, eof);

                    if (expr) {
                        /** configuration for pretty-printing **/
                        ppconfig ppc;
                        ppstate_standalone pps(&cout, 0, &ppc);

                        //pps.prettyn(expr);

                        // TODO:
                        auto [ value, scm_error ] = this->vsm_.eval(expr);

                        if (scm_error.is_error()) {
                            /* print error */

                            cout << "scm error: " << scm_error.what() << endl;
                            cout << "top-level expression: " << expr << endl;
                        } else {
                            /* print value */

                            cout << "scm result:" << endl;
                            pps.pretty(value);
                        }

                    } else if (error.is_error()) {
                        cout << "parsing error (detected in " << error.src_function() << "): " << endl << endl;
                        error.report(cout);

                        /* discard stashed remainder of input line
                         * (for nicely-formatted errors)
                         */
                        rdr.reset_to_idle_toplevel();
                        break;
                    }

                    input = input.after_prefix(consumed);
                    parser_stack_size = psz;
                }

                /* here: input.empty() or error encountered */

            }

            auto [expr, _1, _2, error] = rdr.read_expr(input, true /*eof*/);

            if (expr) {
                ppconfig ppc;
                ppstate_standalone pps(&cout, 0, &ppc);

                pps.prettyn<rp<Expression>>(rp<Expression>(expr));
            } else if (error.is_error()) {
                cout << "parsing error (detected in " << error.src_function() << "): " << endl;
                error.report(cout);
            }

            rx.history_save("repl_history.txt");
        }

        // ----- Schematika -----

        Schematika::Schematika(const Config & cfg) :
            p_impl_{new Impl(cfg)}
        {}

        Schematika::~Schematika()
        {}

        Schematika
        Schematika::make(const Config & cfg)
        {
            return Schematika(cfg);
        }

        void
        Schematika::interactive_repl()
        {
            p_impl_->interactive_repl();
        }
    }
}

/* end Schematika.cpp */
