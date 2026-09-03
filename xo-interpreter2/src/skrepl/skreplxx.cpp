/** @file skreplxx.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include <xo/interpreter2/init_interpreter2.hpp>
#include <xo/interpreter2/cx/Interpreter2Appcx.hpp>
#include <xo/interpreter2/VirtualSchematikaMachine.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/init_facet.hpp>
#include <xo/facet/cx/FacetAppcx.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/indentlog2/appcx_indentlog2.hpp>
#include <xo/indentlog2/config_indentlog2.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <replxx.hxx>
#include <iostream>
#ifdef __APPLE__
# include <unistd.h> // for STDIN_FILENO on OSX
#endif

namespace xo {
    using xo::pp::scope;
    using xo::scm::DVirtualSchematikaMachine;
    using xo::scm::VsmResultExt;
    //using xo::pp::ThreadPrettySink;
    using xo::pp::PpConfig;
    using xo::mm::AAllocator;
    using xo::mm::ArenaConfig;
    using xo::mm::DArena;
    using span_type = xo::mm::span<const char>;
    using xo::facet::FacetRegistry;
    using xo::facet::TypeRegistry;
    using std::clog;
    using std::cerr;

    // presumeably replxx assumes input is a tty anyway?
    //
    bool replxx_getline(bool interactive,
                        bool is_at_toplevel,
                        replxx::Replxx & rx,
                        span_type * p_input
                        //const char ** p_input
                        )
    {
        using namespace std;

        char const * prompt = "";

        if (interactive) {
            prompt = ((is_at_toplevel) ? "> " : ". ");
        }

        const char * input_cstr = rx.input(prompt);

        bool retval = (input_cstr != nullptr);

        if (retval)
            *p_input = span_type::from_cstr(input_cstr);

        if (input_cstr)
            rx.history_add(input_cstr);

        return retval;
    }

    void
    welcome(std::ostream & os)
    {
        using namespace std;

        os << "schematika repl" << endl;
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

    struct ReplConfig {
        ReplConfig() = default;

        std::size_t max_history_size_ = 1000;
        std::string repl_history_fname_ = "skrepl_history.txt";
        bool debug_flag_ = false;
    };

    struct MyAppConfig {
        using VsmConfig = xo::scm::VsmConfig;

        //using ReaderConfig = xo::scm::ReaderConfig;
        //using X1CollectorConfig = xo::mm::X1CollectorConfig;
        //using ArenaConfig = xo::mm::ArenaConfig;

        MyAppConfig(const ReplConfig & repl_cfg = ReplConfig(),
                    const ArenaConfig & app_arena_cfg = ArenaConfig().with_name("skreplxx").with_size(32 * 1024),
                    const VsmConfig & vsm_cfg = VsmConfig().with_x1_config(VsmConfig::std_x1_config().with_debug_flag(true).with_sanitize_flag(true)))
        : repl_config_{repl_cfg}, app_arena_config_{app_arena_cfg}, vsm_config_{vsm_cfg}
        {
            //rdr_config_.reader_debug_flag_ = true;
            //rdr_config.parser_debug_flag_ = true;
            //rdr_config.tk_debug_flag_ = true;
        }

        ReplConfig repl_config_;
        ArenaConfig app_arena_config_;
        VsmConfig vsm_config_;
        //ReaderConfig rdr_config_;
        //X1CollectorConfig x1_config_ = (X1CollectorConfig().with_name("gc").with_size(4*1024*1024));
        //ArenaConfig fixed_config_ = (ArenaConfig().with_name("fixed").with_size(4*1024));
    };

    struct MyApp {
        //using AAllocator = xo::mm::AAllocator;
        //using DX1Collector = xo::mm::DX1Collector;
        //using X1CollectorConfig = xo::mm::X1CollectorConfig;
        using AGCObject = xo::mm::AGCObject;
        //using DArena = xo::mm::DArena;
        //using ArenaConfig = xo::mm::ArenaConfig;
        using VsmConfig = xo::scm::VsmConfig;
        using Replxx = replxx::Replxx;
        using span_type = DVirtualSchematikaMachine::span_type;

        MyApp(const MyAppConfig & cfg = MyAppConfig())
            : repl_config_{cfg.repl_config_},
              app_arena_{cfg.app_arena_config_},
              vsm_config_{cfg.vsm_config_}
        {
            this->interactive_ = isatty(STDIN_FILENO);

            rx_.set_max_history_size(repl_config_.max_history_size_);
            rx_.history_load(repl_config_.repl_history_fname_);
            //    rx.bind_key_internal(Replxx::KEY::control('p'), "history_previous");
            //    rx.bind_key_internal(Replxx::KEY::control('n'), "history_next");
        }

        /** borrows calling thread to run application **/
        void run();

    private:
        void _init();
        void _start();
        void _repl();
        bool _read_eval_print(span_type * p_input, bool eof);
        void _stop();

    private:
        InitEvidence init_evidence_ = 0;
        ReplConfig repl_config_;
        bool interactive_ = false;
        Replxx rx_;
        ///** collector/allocator for schematika expressions **/
        //DX1Collector x1_;
        ///** e.g. for DArenaHashMap within global symtab **/
        //DArena fixed_;

        /** arena with same lifetime as this application **/
        DArena app_arena_;
        /** schematika virtual machine **/
        VsmConfig vsm_config_;
        abox<AGCObject,DVirtualSchematikaMachine> vsm_;
    };

    void
    MyApp::run()
    {
        this->_init();
        this->_start();
        this->_repl();
    }

    void
    MyApp::_init()
    {
        using xo::S_facet_tag;
        using xo::S_indentlog2_tag;
        using xo::FacetConfig;
        using xo::Indentlog2_Config;
        using xo::AppContext;
        using xo::AppConfig;
        using ReplAppConfig = AppConfig<S_indentlog2_tag, S_facet_tag, S_interpreter2_tag>;
        using ReplAppContext = AppContext<S_indentlog2_tag, S_facet_tag, S_interpreter2_tag>;

        ReplAppConfig repl_config{
            Indentlog2_Config(PpConfig().with_logbuf_config
                              (ArenaConfig().with_size(1024 * 1024)),
                              64 * 1024 /*c_temp_arena_capacity*/),
            FacetConfig(1024 /*c_facet_registry_capacity*/,
                        1024 /*c_type_registry_capacity*/),
            Interpreter2Config()
        };
        ReplAppContext utest_appcx{repl_config};

        Subsystem::initialize_all();

        vsm_.adopt(DVirtualSchematikaMachine::make(obj<AAllocator,DArena>(&app_arena_),
                                                   vsm_config_,
                                                   obj<AAllocator,DArena>(&app_arena_)));
    }

    void
    MyApp::_start()
    {
        welcome(cerr);

        vsm_->begin_interactive_session();
    }

    void
    MyApp::_repl()
    {
        bool eof = false;
        span_type input;

        // outer loop: fetch one line of interactive input
        while (replxx_getline(interactive_, vsm_->is_at_toplevel(), rx_, &input)) {

            // inner loop: consume up to one expression at a time.
            while (!input.empty() && this->_read_eval_print(&input, false /*eof*/))
                {
                    ;
                }

            /* here: either:
             * 1. input.empty() or
             * 2. error encountered
             */
        }

        /* reminder: eof can complete at most one token */
        this->_read_eval_print(&input, true /*eof*/);
    }

    /** body of read-parse-print loop
     *
     *  true -> no errors;
     *  false -> reader encountered error
     **/
    bool
    MyApp::_read_eval_print(span_type * p_input,
                            bool eof)
    {
        scope log(XO_DEBUG_(repl_config_.debug_flag_));

        if (!p_input || p_input->empty())
            return true;

        VsmResultExt res = vsm_->read_eval_print(*p_input, eof);

        *p_input = res.remaining_;

        return !res.is_error();
    }

    void
    MyApp::_stop()
    {
        vsm_._drop();
    }

} /*namespace xo*/

int
main (int argc, char * argv[])
{
    using xo::MyAppConfig;
    using xo::MyApp;

    MyAppConfig cfg;
    // [cmdline options here]

    cfg.vsm_config_.x1_config_.debug_flag_ = true;

    MyApp app(cfg);

    app.run();
} /*main*/

/* end skreplxx.cpp */
