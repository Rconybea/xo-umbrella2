/** @file AppContext.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Compose an application context from heterogeneous subsystems, without
 *  runtime polymorphism.
 *
 *  Each subsystem contributes its own context type; the composed AppContext
 *  owns one of each.  Lookup is a base-class conversion, so it costs nothing
 *  at runtime: no vtable, no type erasure, no search.
 *
 *  A subsystem's context is constructed with access to the contexts of the
 *  subsystems it depends on.  A context opts in by providing a constructor
 *  templated on the context of everything below it:
 *
 *      struct FooCx {
 *          template <typename Deps>
 *          explicit FooCx(Deps & deps)
 *              : quux_{&deps.template cx<S_quux_tag>()} {}
 *          ...
 *      };
 *
 *  A context with no dependencies is default-constructed instead, so leaf
 *  subsystems need no boilerplate.
 *
 *  @p Deps is the context composed from the subsystems BELOW this one -- not
 *  the whole application.  That is deliberate: a context cannot reach upward,
 *  so the dependency direction is enforced by the type rather than by
 *  convention.
 *
 *  Ordering is structural rather than promised.  AppContext<Tags...> builds a
 *  chain in which the contexts of lower-level subsystems form a BASE of the
 *  holder for each higher-level one.  A base is fully constructed before the
 *  derived part, so @p deps always refers to complete objects -- we never
 *  reach sideways into a partially constructed sibling.
 *
 *  @p Tags... is given bottom-up (dependencies first).  That is the order
 *  already maintained for this tree: see xo_emit_dependency_edges() and
 *  xo-cmake/etc/xo/subsystem-list.
 **/

#pragma once

#include <concepts>
#include <type_traits>

namespace xo {
    /** Participating subsystems specialize this for their unique subsystem tag,
     *  providing their own Type alias
     **/
    template <typename SubsystemTag>
    class SubsystemContext {
    public:
        using Type = void;
    };

    /**
     * @brief Contribution to application state from a specific subsystem.
     * @tp SubsystemTag identifies the subsystem
     *
     * Owns that subsystem's context.  AppContext inherits one of these per
     * participating subsystem, which is what makes lookup a base-class
     * conversion instead of a search.
     **/
    template <typename SubsystemTag>
    class SubsystemContextHolder {
    public:
        using ContextType = SubsystemContext<SubsystemTag>::Type;

        /* NB reported here rather than at the point of use: forming the return
         * type of AppContext::cx() instantiates this holder, so a missing
         * specialization would otherwise surface as
         *   "field has incomplete type 'void'"
         */
        static_assert(!std::is_void_v<ContextType>,
                      "xo::SubsystemContextHolder: subsystem tag has no"
                      " SubsystemContext<Tag>::Type."
                      " Specialize SubsystemContext<> for this tag,"
                      " or omit the tag from the AppContext composition");

        /** dependent subsystem: hand it the contexts below it **/
        template <typename Deps>
            requires std::constructible_from<ContextType, Deps &>
        explicit SubsystemContextHolder(Deps & deps) : cx_{deps} {}

        /** leaf subsystem: nothing below it that it wants **/
        template <typename Deps>
            requires (!std::constructible_from<ContextType, Deps &>)
        explicit SubsystemContextHolder(Deps &) : cx_{} {}

        /** this subsystem's contribution to application state **/
        ContextType cx_;
    };

    template <typename SubsystemTag>
    using SubsystemContextType = SubsystemContextHolder<SubsystemTag>::ContextType;

    namespace detail {
        /** dependent false: lets a static_assert in a template body fire only
         *  on instantiation.  NB not sizeof(Tag)==0 -- subsystem tags are
         *  usually incomplete types (forward-declared), so sizeof would be a
         *  hard error before the assert could speak.
         **/
        template <typename...>
        inline constexpr bool always_false_v = false;

        /** never defined.  Names the mistake in the follow-on diagnostic when
         *  a context asks for a subsystem that is not below it.
         **/
        template <typename Tag>
        struct subsystem_is_not_below_this_one;

        /** chain of subsystem contexts, HIGHEST-level first.
         *
         *  Chain<Head, Tail...> derives from Chain<Tail...>, so the lower-level
         *  subsystems are a base -- constructed first, and complete by the time
         *  Head's holder is constructed.
         **/
        template <typename... Tags>
        class ContextChain;

        template <>
        class ContextChain<> {
        public:
            /** bottom of the chain: every lookup here is a subsystem making
             *  forbidden attempt to ask for something outside its
             *  dependency set.
             **/
            template <typename Tag>
            subsystem_is_not_below_this_one<Tag> & cx() const noexcept {
                static_assert(always_false_v<Tag>,
                              "xo::AppContext: this subsystem's context asked"
                              " for a subsystem that is not BELOW it."
                              " A context may only use its direct or indirect dependencies."
                              " Check tag order, must be in bottom-up order");
            }
        };

        template <typename Head, typename... Tail>
        class ContextChain<Head, Tail...>
            : public ContextChain<Tail...>,             /* lower levels: built first */
              public SubsystemContextHolder<Head>       /* then this one */
        {
        public:
            /** contexts of every subsystem below @tp Head **/
            using PrefixType = ContextChain<Tail...>;

            ContextChain()
                : PrefixType{},
                  /* legal: PrefixType is a base whose construction has
                   * completed, so converting `this` to it is well-defined
                   * ([class.cdtor]).  Verified clean under UBSan.
                   */
                  SubsystemContextHolder<Head>{ static_cast<PrefixType &>(*this) }
                {}

            template <typename Tag>
            SubsystemContextType<Tag> & cx() noexcept {
                return static_cast<SubsystemContextHolder<Tag> *>(this)->cx_;
            }

            template <typename Tag>
            const SubsystemContextType<Tag> & cx() const noexcept {
                return static_cast<const SubsystemContextHolder<Tag> *>(this)->cx_;
            }
        };

        /* reverse a tag pack: the public API takes tags bottom-up (matching
         * subsystem-list), while ContextChain wants them highest-first
         */
        template <typename... Tags> struct tag_list {};

        template <typename Remaining, typename... Acc>
        struct reverse_tags;

        template <typename... Acc>
        struct reverse_tags<tag_list<>, Acc...> {
            using type = ContextChain<Acc...>;
        };

        template <typename Head, typename... Tail, typename... Acc>
        struct reverse_tags<tag_list<Head, Tail...>, Acc...> {
            using type = reverse_tags<tag_list<Tail...>, Head, Acc...>::type;
        };

        template <typename... Tags>
        using chain_for = reverse_tags<tag_list<Tags...>>::type;
    } /*namespace detail*/

    /** @brief application state, composed from the subsystems named by @tp Tags
     *
     *  Require: @tp Tags in dependency order, lower-level subsystems FIRST.
     *
     *  Use:
     *    using MyApp = AppContext<S_quux_tag, S_bar_tag, S_foo_tag>;
     *
     *    MyApp app;                              // contexts built bottom-up
     *    app.cx<S_foo_tag>().some_foo_state_;
     **/
    template <typename... Tags>
    class AppContext : public detail::chain_for<Tags...> {
    public:
        using ChainType = detail::chain_for<Tags...>;

        AppContext() = default;

        /** context for subsystem @tp Tag.
         *
         *  Resolved at compile time by conversion to the corresponding base
         *  subobject.  A tag absent from @tp Tags... is a compile error.
         **/
        template <typename Tag>
        SubsystemContextType<Tag> & cx() noexcept {
            static_assert((std::is_same_v<Tag, Tags> || ...),
                          "xo::AppContext: subsystem tag is not part of this"
                          " composition");

            return ChainType::template cx<Tag>();
        }

        template <typename Tag>
        const SubsystemContextType<Tag> & cx() const noexcept {
            static_assert((std::is_same_v<Tag, Tags> || ...),
                          "xo::AppContext: subsystem tag is not part of this"
                          " composition");

            return ChainType::template cx<Tag>();
        }
    }; /*AppContext*/
} /*namespace xo*/

/* end AppContext.hpp */
