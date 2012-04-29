#ifndef __METAPROGRAMMING_CONTAINER__
#define __METAPROGRAMMING_CONTAINER__

#include "element_access.h"
#include "type_traits.h"

namespace robot { namespace container
{
    namespace details
    {
        namespace m = metaprogramming;

        template <typename T>
        struct element_value
        {
            T value;
        };

        struct nothing {};

        template <typename T>
        using element_storage =
        m::at_key
        <
            m::is_const<T>,
            m::pair<std::true_type , nothing>,
            m::pair<std::false_type, element_value<T>>
        >;

        template <typename ...>
        struct element;

        template <typename T, typename ...Signature>
        struct element<T, Signature...>: public element_storage<T>
        {};

        template <typename Key, typename T, typename ...Signature>
        struct element<m::pair<Key, T>, Signature...>: public element_storage<T>
        {};

        template <>
        struct element<>
        {};
    }

    template <typename ...>
    struct container;

    template <>
    struct container<> {};

    template <typename Head, typename ...Tail>
    struct container<Head, Tail...>:
        public container<Tail...>,
        public details::element<Head, Tail...>
    {};

    namespace element_access
    {
        namespace m = metaprogramming;

        template <typename, typename>
        struct access;

        template <typename ...ElementSignature, typename Head, typename ...Tail>
        struct access<m::sequence<ElementSignature...>, m::sequence<Head, Tail...>>:
            public access<m::sequence<ElementSignature...>, m::sequence<Tail...>>
        {};

        template <typename Head, typename ...Tail>
        struct access<m::sequence<Tail...>, m::sequence<Head, Tail...>>
        {
            using type = details::element<Head, Tail...>;
        };

        template <typename ...ElementSignature>
        struct access<m::sequence<ElementSignature...>, m::sequence<>>
        {
            static_assert
            (
                true,
                "robot::container::element_access::access: element not found"
            );
        };

        template <typename, typename>
        struct accessor;

        template <typename ...ElementSignature, typename ...Args>
        struct accessor<m::sequence<ElementSignature...>, container<Args...>>
        {
            using type =
            typename
            access
            <
                m::sequence<ElementSignature...>,
                m::sequence<Args...>
            >::type;
        };

        template <size_t n, typename ...Args> struct signature_at_c;
     
        template <typename Head, typename ...Tail>
        struct signature_at_c<0, Head, Tail...>
        {
            using type = m::sequence<Head, Tail...>;
        };
         
        template <size_t n, typename Head, typename ...Tail>
        struct signature_at_c<n, Head, Tail...>: public signature_at_c<n - 1, Tail...>
        {
            static_assert
            (
                n <= sizeof...(Tail),
                "robot::container::element_access::signature_at_c: out of range"
            );
        };

        template <size_t n, typename ...Args>
        struct signature_at_c<n, m::sequence<Args...>>: public signature_at_c<n, Args...> {};

        template <typename ...Args>
        struct signature_at_c<0, m::sequence<Args...>>: public signature_at_c<0, Args...> {};

        template <typename Key, typename ...Args> struct signature_at_key;

        template <typename Head, typename ...Tail>
        struct signature_at_key<typename m::convert_to_pair<Head>::first, Head, Tail...>
        {
            using type = m::sequence<Head, Tail...>;
        };

        template <typename Key, typename Head, typename ...Tail>
        struct signature_at_key<Key, Head, Tail...>: public signature_at_key<Key, Tail...> {};

        template <typename Key, typename... Args>
        struct signature_at_key<Key, m::sequence<Args...>>: public signature_at_key<Key, Args...> {};

        template <typename>
        struct element_;

        template <typename ...Signature>
        struct element_<m::sequence<Signature...>>
        {
            using type = details::element<Signature...>;
        };

        template <typename T>
        using element = typename element_<T>::type;

        template <size_t C, typename ...Args>
        using at_c = m::value_type<m::at_c<C, Args...>>;

        template <typename Key, typename ...Args>
        using at_key = m::value_type<m::at_key<Key, Args...>>;
    }
    
    template <size_t C, typename ...Args>
    element_access::at_c<C, Args...>& at_c(container<Args...>& p)
    {
        using namespace element_access;
        element<typename signature_at_c<C, Args...>::type> *r = &p;
        return r->value;
    }

    template <size_t C, typename ...Args>
    const element_access::at_c<C, Args...>& at_c(const container<Args...>& p)
    {
        using namespace element_access;
        element<typename signature_at_c<C, Args...>::type> *r = &p;
        return r->value;
    }

    template <typename Key, typename ...Args>
    element_access::at_key<Key, Args...>& at_key(container<Args...>& p)
    {
        using namespace element_access;
        element<typename signature_at_key<Key, Args...>::type> *r = &p;
        return r->value;
    }
    
    template <typename Key, typename ...Args>
    const element_access::at_key<Key, Args...>& at_key(const container<Args...>& p)
    {
        using namespace element_access;
        element<typename signature_at_key<Key, Args...>::type> *r = &p;
        return r->value;
    }
}}

#endif //__METAPROGRAMMING_CONTAINER__

