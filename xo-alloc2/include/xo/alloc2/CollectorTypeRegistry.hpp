/** @file CollectorTypeRegistry.hpp
 *
 *  @brief Runtime type registration for gc-aware types
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "Collector.hpp"
#include <functional>

namespace xo {
    namespace mm {
        /** @class CollectorTypeRegistry
         *
         *  @brief Runtime registry for gc-aware types
         *
         *  Singleton to remember known gc-aware types;
         *  use to simplify registering such types
         *  with a collector instance.
         *
         *  Remark: splitting work here between
         *  1. static initializer work: tracking gc-aware types,
         *  2. runtime post-configuration work: report
         *     gc-aware types to GC instances
         *
         *  Use:
         *  1. subsystem foo provides function foo_register_types(obj<ACollector> gc)
         *     Function calls
         *       gc.install_type(impl_for<AGCObject, DQuux>())
         *     for each gc-aware type provided by subsystem foo
         *
         *     Example: in file xo-object2/src/object2/object2_register_types.cpp, see
         *       object2_register_types()
         *
         *  2. during subsystem init, call
         *       CollectorTypeRegistry::instance().register_types(&foo_register_types);
         *
         *     Example: in file xo-object2/src/object2/init_object2.cpp, see
         *       InitSubsys<S_object2_tag>::init()
         *
         *  3. during Collector setup, call
         *       obj<ACollector> gc = ...;
         *       CollectorTypeRegistry::instance().install_types(gc);
         *
         *    Example: in file xo-object2/utest/X1Collector.test.cpp
         *       TEST_CASE("x1")
         **/
        class CollectorTypeRegistry {
        public:
            using init_function_type = std::function<bool (obj<ACollector>)>;

        public:
            /** singleton instance **/
            static CollectorTypeRegistry & instance();

            /** remember a gc-aware type-registration function **/
            void register_types(init_function_type init_fn);

            /** register known GC-aware types with @p gc.
             *  Calls @c gc.isntall_type() for each
             *  such type.
             **/
            bool install_types(obj<ACollector> gc);

        private:
            /** initialization steps for a new Collector instance **/
            std::vector<init_function_type> init_seq_v_;
        };
    }
}

/* end CollectorTypeRegistry.hpp */
