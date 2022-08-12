#pragma once
//#ifndef SM_BASE_H
//#define SM_BASE_H

// ******************************************************************
// ******************************************************************
struct True_T {
    typedef std::integral_constant<bool, true> type;
};
struct False_T {
    typedef std::integral_constant<bool, true> type;
};

namespace helper {
    struct CollectorBase {};

    template<typename ...Args>
    struct CollectorI{ };
}

template<typename ...Args>
struct Collector : helper::CollectorBase {
    typedef helper::CollectorI<Args...> type;
    static constexpr auto size = sizeof ...(Args);
};

template<typename ...>
struct SizeOfCollector;
template<typename ...As>
struct SizeOfCollector<Collector<As...>> {
    static const int size = sizeof...(As);
};


namespace detail {
    template <typename...>
    struct is_one_of {
        static constexpr bool value = false; //std::is_same<int>;
    };
    template <typename F, typename S, typename... T>
    struct is_one_of<F, S, T...> {
        static constexpr bool value =
            std::is_same<F, S>::value || is_one_of<F, T...>::value;
    };
    template <typename F, typename ... T>
    struct is_one_of_collection;
    template <typename F, typename ... T>
    struct is_one_of_collection<F, Collector<T...>> {
        static constexpr bool value = is_one_of<F, T...>::value;
    };

    // ****
    template <typename ...> struct is_one_of_T;
    template <typename T, typename A, typename ...As>
    struct is_one_of_T<T, Collector<A,As...>> {
        typedef typename is_one_of_T<T, Collector<As...>>::type type;
    };
    template <typename T, typename ...As>
    struct is_one_of_T<T, Collector<T,As...>> {
        typedef True_T type;
    };
    template <typename T>
    struct is_one_of_T<T, Collector<>> {
        typedef False_T type;
    };

    // ***
    template <typename...>
    struct no_duplicates {
        static constexpr bool value = true;
    };
    template <typename S, typename ... T>
    struct no_duplicates<S, T...> {
        static constexpr bool value = no_duplicates<T...>::value &&
                !is_one_of_collection<S, Collector<T...>>::value;
        //no_duplicates<T...>::value &&
    };

    template <typename ... T>
    struct no_duplicates_in_collection;
    template <typename ... T>
    struct no_duplicates_in_collection<Collector<T...>> {
        static constexpr bool value = no_duplicates<T...>::value;
    };
}

// ******************************************************************
// ******************************************************************

namespace detail {
    template <typename ...>
    struct JoinCollectors;
    template <typename ... ElsOne, typename ... ElsTwo, typename ... Cs>
    struct JoinCollectors<Collector<ElsOne...>, Collector<ElsTwo...>, Cs...> {
        typedef typename JoinCollectors<Collector<ElsOne..., ElsTwo...>, Cs...>::type type;
    };
    template <typename ... Els>
    struct JoinCollectors<Collector<Els...>> {
        typedef Collector<Els...> type;
    };

    // Join Two Collectors
    template<typename ...A>
    struct CollectorsJoinTwo;
    template<typename ...As>
    struct CollectorsJoinTwo<Collector<As...>> {
        typedef Collector<As...> type;
    };
    template<typename A, typename ...As, typename ...Bs>
    struct CollectorsJoinTwo< Collector<A, As...>, Collector<Bs...> > {
        typedef Collector<As..., Bs...> type;
    };

    // *****************************************************************
    template<typename ...> struct CollectorsRemoveSecond_Impl;
    template<typename A, typename ...As, typename ...Bs, typename ...RES>
    struct CollectorsRemoveSecond_Impl< Collector<A, As...>, Collector<Bs...> , Collector<RES...>, True_T > {
        typedef typename CollectorsRemoveSecond_Impl< Collector<As...>, Collector<Bs...> , Collector<RES...>>::type type;
    };
    template<typename A, typename ...As, typename ...Bs, typename ...RES>
    struct CollectorsRemoveSecond_Impl< Collector<A, As...>, Collector<Bs...> , Collector<RES...>, False_T > {
        typedef typename CollectorsRemoveSecond_Impl< Collector<As...>, Collector<Bs...> , Collector<RES..., A>>::type type;
    };
    template<typename A, typename ...As, typename ...Bs, typename ...RES>
    struct CollectorsRemoveSecond_Impl< Collector<A, As...>, Collector<Bs...> , Collector<RES...> > {
        typedef typename CollectorsRemoveSecond_Impl< Collector<A, As...>, Collector<Bs...>, Collector<RES...>, is_one_of_T<A, Collector<Bs...>>>::type type;
    };
    template<typename ...Bs, typename ...RES>
    struct CollectorsRemoveSecond_Impl< Collector<>, Collector<Bs...> , Collector<RES...> > {
        typedef Collector<RES...> type;
    };

    template<typename ...> struct CollectorsRemoveSecond;
    template<typename ...As, typename ...Bs>
    struct CollectorsRemoveSecond< Collector<As...>, Collector<Bs...> > {
        typedef CollectorsRemoveSecond_Impl<Collector<As...>, Collector<Bs...>, Collector<>> type;
    };

    // *****************************************************************
    template<typename ...>
    struct iterateCollector;
    template<typename F, typename X, typename ...Xs>
    struct iterateCollector<F, Collector<X,Xs...>> {
        void operator()(F &f) {
            if(f.template impl<X>()) {
                iterateCollector<F,Collector<Xs...>> ic;
                ic(f);
            }
        }
    };
    template<typename F>
    struct iterateCollector<F, Collector<>> {
        void operator()(F &) { }
    };
}

// *****************************************************************
// *****************************************************************
#define make_join_lists(XXX, XXXX) \
namespace join_list_helper_ ## XXX { \
    template<typename , typename ...> \
    struct JoinListOfCollectors_impl; \
    template<typename X, typename ...A> \
    struct JoinListOfCollectors_impl { \
        static_assert(std::is_base_of<helper::CollectorBase, typename X::XXXX>::value, "SM: JoinListOfCollectors only takes types with a member-type named type derived from CollectorBase."); \
        typedef typename detail::CollectorsJoinTwo<typename X::XXXX, typename JoinListOfCollectors_impl<A...>::type >::type type; \
    }; \
    template<typename X> \
    struct JoinListOfCollectors_impl<X> { \
        static_assert(std::is_base_of<helper::CollectorBase, typename X::XXXX>::value, "JoinListOfCollectors only takes types derived from CollectorBase."); \
        typedef typename X::XXXX type; \
    }; \
} \
template<typename ...A> \
struct JoinListOfCollectors_ ## XXX { \
    typedef typename join_list_helper_ ## XXX::JoinListOfCollectors_impl<A...>::type type; \
}

namespace detail {
    make_join_lists(states, details::states);
    make_join_lists(type, type);
}

// ******************************************************************
// ******************************************************************
#define make_tester_func_type(XXX) \
namespace detail { \
    template <typename T, typename Type=typename T::XXX> \
    struct has_type_ ## XXX ## _helper; \
    template <typename T> std::true_type  has_type_ ## XXX ## _test(has_type_ ## XXX ## _helper<T> *); \
    template <typename T> std::false_type has_type_ ## XXX ## _test(...); \
} \
template <typename T> \
struct has_type_ ## XXX : decltype(detail::has_type_ ## XXX ## _test<T>(nullptr)) { }

// *****************************************************************
#define make_tester_func_static(XXX, FUNCTYPE) \
template<typename T> \
struct has_static_ ## XXX { \
    template<typename U, U> struct Check; \
    template<typename U> static std::true_type Test(Check<FUNCTYPE, &U::XXX>*); \
    template<typename U> static std::false_type Test(...); \
    static const bool value = decltype(Test<T>(0))::value; \
}

// ******************************************************************
#define make_tester_func_details_type(XXX) \
namespace detail { \
    template <typename T, typename Type=typename T::details::XXX> \
    struct has_type_details_ ## XXX ## _helper; \
    template <typename T> std::true_type  has_type_details_ ## XXX ## _test(has_type_details_ ## XXX ## _helper<T> *); \
    template <typename T> std::false_type has_type_details_ ## XXX ## _test(...); \
} \
template <typename T> \
struct has_type_details_ ## XXX : decltype(detail::has_type_details_ ## XXX ## _test<T>(nullptr)) { }

// ******************************************************************
namespace detail {
    make_tester_func_static(name, const char *(*)());
    make_tester_func_static(description, const char *(*)());

    make_tester_func_type(details);

    make_tester_func_details_type(TransitionsT);
}

//#endif // SM_BASE_H
