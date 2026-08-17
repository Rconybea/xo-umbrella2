/** @file Schematika.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "Schematika.hpp"
#include "BuiltinPrimitives.hpp"
#include "GlobalEnv.hpp"
#include "VirtualSchematikaMachine.hpp"
#include <xo/reader/reader.hpp>
#include <xo/alloc/alloc_ostream.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/refcnt/Refcounted_ostream.hpp>
#include <replxx.hxx>
#include <ostream>
#include <unistd.h> // for STDIN_FILENO on OSX

namespace xo {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::pp::scope;
    using xo::gc::IAlloc;
    using xo::gc::GC;
    using replxx::Replxx;
    using namespace std;

    namespace {
#ifdef OBSOLETE
        /** render @p expr with line breaking, as legacy ppstate_standalone did **/
        template <typename T>
        void render_expr(std::ostream & os, const T & expr) {
            auto pps = PrettySink(PpConfig::scratch_aux("schematika.", 135, PpStyle::colored()),
                                  os.rdbuf());

            pps.pp(expr);
            os << std::endl;

            //os << pp.output() << std::endl;
        }
#endif
    } /*namespace*/

    namespace scm {

        class Schematika::Impl {
        public:
            /** note: choosing to have Schemtika::Impl
             *        rather than VirtualSchematikaMachine to own allocator
             *        to preserve option to share it
             **/
            Impl(const Config & config, up<IAlloc> mm, gp<GlobalEnv> toplevel_env);
            ~Impl();

            /** create instance + allocator **/
            static up<Impl> make(const Config & cfg);

            /** borrow calling thread to run interactive read-eval-print loop;
             *  input from stdin, output to stdout.
             **/
            void interactive_repl();

            void welcome(std::ostream & os);

            /** get one line of input. prompt if @p interactive,
             *  with prompt depending on @p parser_stack_size.
             *  Use @p rx to perform line editing (when @p interactive).
             *  Store completed line in @p input.
             **/
            bool replxx_getline(bool interactive,
                                std::size_t parser_stack_size,
                                replxx::Replxx & rx,
                                std::string & input);

        private:
            /** configuration **/
            Config config_;
            /** ownership for memory allocator / garbage collector;
             *  @ref vsm_ holds naked pointer, so this could in principle be nullptr
             *  in case want to maintain allocator ownership from outside.
             *
             *  note: must appear before @ref vsm_, so latter gets destroyed first
             **/
            up<IAlloc> mm_;
            /** schematika interpreter **/
            VirtualSchematikaMachine vsm_;
        };

        Schematika::Impl::Impl(const Config & config, up<IAlloc> mm, gp<GlobalEnv> toplevel_env) :
            config_{config},
            mm_{std::move(mm)},
            vsm_{mm_.get(), toplevel_env, config.vsm_log_level_}
        {

        }

        Schematika::Impl::~Impl() = default;

        up<Schematika::Impl>
        Schematika::Impl::make(const Config & cfg)
        {
            up<IAlloc> mm = GC::make(cfg.gc_config_);
            rp<GlobalSymtab> symtab = GlobalSymtab::make_empty();
            gp<GlobalEnv> env = GlobalEnv::make_empty(mm.get(), symtab);

            /* also see VirtualSchematikaMachineFlyweight::Impl::Impl,
             * for BuiltinPrimitives::install_interpreter_conversions()
             */
            BuiltinPrimitives::install(mm.get(), env);

            return std::make_unique<Impl>(cfg, std::move(mm), env);
        }

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
            scope log(XO_DEBUG_(true));

            using span_type = xo::scm::span<const char>;

            bool interactive = isatty(STDIN_FILENO);

            Replxx rx;
            rx.set_max_history_size(config_.history_size);
            rx.history_load(config_.history_file);
            //    rx.bind_key_internal(Replxx::KEY::control('p'), "history_previous");
            //    rx.bind_key_internal(Replxx::KEY::control('n'), "history_next");

            reader rdr(vsm_.toplevel_env()->symtab(), config_.debug_flag);
            rdr.begin_interactive_session();

            string input_str;

            bool eof = false;

            span_type input;
            std::size_t parser_stack_size = 0;

            PrettySink pps(PpConfig::scratch_aux("skpp.", 135, PpStyle::colored()),
                           cout.rdbuf());

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
                        //render_expr(cout, expr);

                        // TODO:
                        auto [ value, scm_error ] = this->vsm_.toplevel_eval(expr);

                        if (scm_error.is_error()) {
                            /* print error */

                            cout << "scm error: " << scm_error.what() << endl;
                            cout << "top-level expression: " << expr << endl;
                        } else {
                            /* print value */

                            cout << "scm result:" << endl;
                            cout << value << endl;
                            //pps.pretty(value);
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

                cerr << endl;
            }

            auto [expr, _1, _2, error] = rdr.read_expr(input, true /*eof*/);

            if (expr) {
                pps.pp(rp<Expression>(expr));
                pps.complete();
                //render_expr(cout, rp<Expression>(expr));
            } else if (error.is_error()) {
                cout << "parsing error (detected in " << error.src_function() << "): " << endl;
                error.report(cout);
            }

            rx.history_save("repl_history.txt");
        }

        // ----- Schematika -----

        Schematika::Schematika(const Config & cfg) : p_impl_{Impl::make(cfg)}
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
