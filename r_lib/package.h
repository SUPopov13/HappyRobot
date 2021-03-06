#ifndef __PACKAGE__
#define __PACKAGE__

#include <iostream>
#include <memory>
#include <numeric>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <array>
#include <exception>

#include "metaprogramming/serialization.h"
#include "metaprogramming/select.h"

#include "sequence.h"
#include "phis_value.h"

namespace robot { namespace package_creation
{
    template <typename SizeType, typename ValueType>
    class repeat: public std::vector<ValueType>
    {};

    template <typename ...Args>
    using no_const_args = metaprogramming::select<metaprogramming::is_no_const, Args...>;

    template <typename ...Args>
    using size = metaprogramming::byte_count<size_t, Args...>;

    template <typename ...Args>
    using pattern = metaprogramming::sequence<Args...>;

    namespace serialization
    {
        struct constant_mismatch_error: public std::logic_error
        {
            constant_mismatch_error(): std::logic_error("parser error: const package field mismatch") {}
        };

        template <typename T0, typename T1>
        using pair = metaprogramming::pair<T0, T1>;

        template <bool, typename>
        class size_c_tmp;

        template <bool, typename>
        class serialize_tmp;

        // const size type serialize

        template <typename T>
        class size_c_tmp<true, T>
        {
        public:
            constexpr static size_t size(const T& t) { return robot::package_creation::size<T>::value; }
        };

        // stl container serialize

        template <typename T>
        class stl_compatible_container_size_c
        {
            using val_t = typename T::value_type;
            using cref_t = typename T::const_reference;

            static const bool c = metaprogramming::is_const_size<val_t>::value;

        public:
            static size_t size(const T& t)
            {
                return
                std::accumulate
                (
                    t.begin(),
                    t.end(),
                    0,
                    [](size_t p0, cref_t p1) { return p0 + size_c_tmp<c, val_t>::size(p1); }
                );
            }
        };

        template <> class size_c_tmp<false, std::string>: public stl_compatible_container_size_c<std::string> {};

        template <typename T> class size_c_tmp<false, std::vector<T>>: public stl_compatible_container_size_c<std::vector<T>> {};
        template <typename T> class size_c_tmp<false, std::list  <T>>: public stl_compatible_container_size_c<std::list  <T>> {};

        template <typename Key, typename T> class size_c_tmp<false, std::map<Key, T>>: public stl_compatible_container_size_c<std::map<Key, T>> {};
        template <typename Key, typename T> class size_c_tmp<false, std::set<Key, T>>: public stl_compatible_container_size_c<std::set<Key, T>> {};

        template <typename Key, typename T> class size_c_tmp<false, std::multimap<Key, T>>: public stl_compatible_container_size_c<std::multimap<Key, T>> {};
        template <typename Key, typename T> class size_c_tmp<false, std::multiset<Key, T>>: public stl_compatible_container_size_c<std::multiset<Key, T>> {};

        template <typename T, size_t C> class size_c_tmp<false, std::array<T, C>>: public stl_compatible_container_size_c<std::array<T, C>> {};

        template <typename T, typename Size>
        class size_c_tmp<false, repeat<Size, T>>
        {
            using type = repeat<Size, T>;
        public:
            static size_t size(const type& t)
            {
                return
                size_c_tmp<true, Size>::size(t.size()) +
                size_c_tmp<false, std::vector<T>>::size(t);
            }
        };

        template <typename ValueType, typename Dimension>
        class size_c_tmp<false, phis_value<ValueType, Dimension>>
        {
            using type = phis_value<ValueType, Dimension>;
        public:
            constexpr static size_t size(const type& t)
            {
                return size_c_tmp<true, ValueType>::size(t.get());
            }
        };

        template <typename T>
        class size_c_tmp_wrapper:
            public size_c_tmp<metaprogramming::is_const_size<T>::value, T>
        {};

        template <typename T0, typename T1>
        class size_c_tmp_wrapper<pair<T0, T1>>:
            public size_c_tmp<metaprogramming::is_const_size<T1>::value, T1>
        {};

        template <typename T>
        class serialize_tmp<true, T>
        {
        public:
            static void   serialize(uint8_t *pos, const T& t) { *reinterpret_cast<T*>(pos) = t; }
            static void deserialize(const uint8_t *pos, T& t) { t = *reinterpret_cast<const T*>(pos); }
        };

        template <typename T>
        class stl_compatible_container_serialize 
        {
            using val_t = typename T::value_type;
            using ref_t = typename T::reference;
            using cref_t = typename T::const_reference;

            using size_calc = size_c_tmp_wrapper<val_t>;

        public:
            static void serialize(uint8_t *pos, const T& t)
            {
                for(cref_t x : t) {
                    serialize_tmp<std::is_fundamental<val_t>::value, val_t>::serialize(pos, x);
                    pos += size_calc::size(x);
                }
            }

            static void deserialize(const uint8_t *pos, T& t)
            {
                for(ref_t x : t) {
                    serialize_tmp<std::is_fundamental<val_t>::value, val_t>::deserialize(pos, x);
                    pos += size_calc::size(x);
                }
            }
        };

        template <> class serialize_tmp<false, std::string>: public stl_compatible_container_serialize<std::string> {};

        template <typename T> class serialize_tmp<false, std::vector<T>>: public stl_compatible_container_serialize<std::vector<T>> {};
        template <typename T> class serialize_tmp<false, std::list  <T>>: public stl_compatible_container_serialize<std::list  <T>> {};

        template <typename Key, typename T> class serialize_tmp<false, std::map<Key, T>>: public stl_compatible_container_serialize<std::map<Key, T>> {};
        template <typename Key, typename T> class serialize_tmp<false, std::set<Key, T>>: public stl_compatible_container_serialize<std::set<Key, T>> {};

        template <typename Key, typename T> class serialize_tmp<false, std::multimap<Key, T>>: public stl_compatible_container_serialize<std::multimap<Key, T>> {};
        template <typename Key, typename T> class serialize_tmp<false, std::multiset<Key, T>>: public stl_compatible_container_serialize<std::multiset<Key, T>> {};

        template <typename T, size_t C> class serialize_tmp<false, std::array<T, C>>: public stl_compatible_container_serialize<std::array<T, C>> {};

        template <typename T, typename Size>
        class serialize_tmp<false, repeat<Size, T>>
        {
            using type = repeat<Size, T>;
        public:
            static void serialize(uint8_t *pos, const type& t)
            {
                serialize_tmp<true, Size>::serialize(pos, t.size());
                pos += size_c_tmp_wrapper<Size>::size(t.size());
                serialize_tmp<false, std::vector<T>>::serialize(pos, t);
            }

            static void deserialize(const uint8_t *pos, type& t)
            {
                Size s;
                serialize_tmp<true, Size>::deserialize(pos, s);
                t.resize(s);
                pos += size_c_tmp_wrapper<Size>::size(s);
                serialize_tmp<false, std::vector<T>>::deserialize(pos, t);
            }
        };

        template <typename ValueType, typename Dimension>
        class serialize_tmp<false, phis_value<ValueType, Dimension>>
        {
            using type = phis_value<ValueType, Dimension>;
        public:
            static void serialize(uint8_t *pos, const type& t)
            {
                serialize_tmp<true, ValueType>::serialize(pos, t.get());
            }

            static void deserialize(const uint8_t *pos, type& t)
            {
                ValueType v;
                serialize_tmp<true, ValueType>::deserialize(pos, v);
                t.set(v);
            }
        };

        template <typename T>
        class serialize_tmp_wrapper:
            public serialize_tmp<std::is_fundamental<T>::value, T>
        {};

        template <typename T0, typename T1>
        class serialize_tmp_wrapper<pair<T0, T1>>:
            public serialize_tmp<std::is_fundamental<T1>::value, T1>
        {};

        template <typename ...>
        class serialize_element;

        template <typename ...>
        class serializer;

        template <typename T, typename ...Tail>
        class serialize_element<T, pattern<Tail...>>:
            protected size_c_tmp_wrapper<T>,
            protected serialize_tmp_wrapper<T>
        {};

        struct virtual_serializer
        {
            virtual void serialize(uint8_t *pos) const = 0;
            virtual void deserialize(const uint8_t *pos) = 0;
            virtual size_t size_c() const  = 0;

            virtual ~virtual_serializer() {};
        };

        template <typename T>
        class serialize_wrapper: public virtual_serializer
        {
            T& val;

        public:
            serialize_wrapper(T& t): val(t) {}

            void serialize(uint8_t *pos) const   { serialize_tmp_wrapper<T>::serialize(pos, val); }
            void deserialize(const uint8_t *pos) { serialize_tmp_wrapper<T>::deserialize(pos, val); }

            size_t size_c() const { return size_c_tmp_wrapper<T>::size(val); }
        };

        class any
        {
            std::shared_ptr<virtual_serializer> v;
        public:
            template <typename T>
            any(T& t): v(new serialize_wrapper<T>(t)) {}

            void serialize(uint8_t *pos) const   { v->serialize(pos); }
            void deserialize(const uint8_t *pos) { v->deserialize(pos); }

            size_t size_c() const { return v->size_c(); }
        };

        template <>
        class size_c_tmp<false, any>
        {
        public:
            static size_t size(const any& t)
            {
                return t.size_c();
            }
        };

        template <>
        class serialize_tmp<false, any>
        {
        public:
            static void serialize(uint8_t *pos, const any& t)
            {
                t.serialize(pos);
            }

            static void deserialize(const uint8_t *pos, any& t)
            {
                t.deserialize(pos);
            }
        };

        template <>
        class serializer<pattern<>, pattern<>>
        {
        protected:
            static void serialize(uint8_t*){}
            static void deserialize(const uint8_t*){}
            static size_t size() { return 0; }
        };

        template <typename ...Args, typename ...FArgs, typename F, F U>
        class serializer<pattern<std::integral_constant<F, U>, Args...>, pattern<FArgs...>>:
            serialize_element<F, pattern<Args...>>,
            serializer<pattern<Args...>, pattern<FArgs...>>
        {
        protected:
            using head = serialize_element<F, pattern<Args...>>;
            using tail = serializer<pattern<Args...>, pattern<FArgs...>>;

            constexpr F current(const pattern<F, Args...>& p) const
            {
                return U;
            }

            static void serialize(uint8_t *dst, const FArgs&... args)
            {
                head::serialize(dst, U);
                tail::serialize(dst + head::size(U), args...);
            }

            static void deserialize(const uint8_t *dst, FArgs&... args)
            {
                F c;
                head::deserialize(dst, c);
                if(c != U)
                    throw constant_mismatch_error();
                tail::deserialize(dst + head::size(U), args...);
            }

            static size_t size(const FArgs&... args)
            {
                return tail::size(args...) + head::size(U);
            }
        };

        template <typename ...Args, typename ...FArgs, typename F>
        class serializer<pattern<F, Args...>, pattern<F, FArgs...>>:
            serialize_element<F, pattern<Args...>>,
            serializer<pattern<Args...>, pattern<FArgs...>>
        {
        protected:
            using head = serialize_element<F, pattern<Args...>>;
            using tail = serializer<pattern<Args...>, pattern<FArgs...>>;

            F current(const pattern<F, Args...>& p) const
            {
                return at_c<0>(p);
            }

            static void serialize(uint8_t *dst, const F& f, const FArgs&... args)
            {
                head::serialize(dst, f);
                tail::serialize(dst + head::size(f), args...);
            }

            static void deserialize(const uint8_t *dst, F& f, FArgs&... args)
            {
                head::deserialize(dst, f);
                tail::deserialize(dst + head::size(f), args...);
            }

            static size_t size(const F& f, const FArgs&... args)
            {
                return tail::size(args...) + head::size(f);
            }
        };

        template <typename ...Args, typename ...FArgs, typename T0, typename T1>
        class serializer<pattern<pair<T0, T1>, Args...>, pattern<FArgs...>>:
            protected serializer<pattern<T1, Args...>, pattern<FArgs...>>
        {};

        template <typename ...Args, typename ...FArgs, typename ...SubSequenceArgs>
        class serializer<pattern<pattern<SubSequenceArgs...>, Args...>, pattern<FArgs...>>:
            protected serializer<pattern<SubSequenceArgs..., Args...>, pattern<FArgs...>>
        {};

        class sequence_serialize
        {
            struct sequence_size
            {
                size_t size;

                sequence_size(): size(0) {}

                template <typename T>
                void operator() (const T& t)
                {
                    size += serialization::size_c_tmp_wrapper<T>::size(t);
                }

                template <typename T, T C>
                void operator() (const std::integral_constant<T, C>& t)
                {
                    size += package_creation::size<T>::value;
                }

                template <typename ...T>
                void operator() (const pattern<T...>& t)
                {
                    sequence_size s;
                    algorithm::for_each(s, t);
                    size += s.size;
                }
            };

        protected:
            template <typename T>
            static size_t calc_sequence_size(const T& t)
            {
                sequence_size s;
                s(t);
                return s.size;
            }

            struct sequence_serializer
            {
                uint8_t* ptr;

                sequence_serializer(uint8_t* p): ptr(p) {}

                template <typename T>
                void operator() (const T& t)
                {
                    serialization::serialize_tmp_wrapper<T>::serialize(ptr, t);
                    ptr += serialization::size_c_tmp_wrapper<T>::size(t);
                }

                template <typename T, T C>
                void operator() (const std::integral_constant<T, C>& t)
                {
                    this->operator()(C);
                }

                template <typename ...T>
                void operator() (const pattern<T...>& t)
                {
                    sequence_serializer s(ptr);
                    algorithm::for_each(s, t);
                    ptr = s.ptr;
                }
            };

            struct sequence_deserializer
            {
                const uint8_t* ptr;

                sequence_deserializer(const uint8_t* p): ptr(p) {}

                template <typename T, T C>
                void operator() (const std::integral_constant<T, C>& t)
                {
                    T c;
                    serialization::serialize_tmp_wrapper<T>::deserialize(ptr, c);
                    if(c != C)
                        throw constant_mismatch_error();
                    this->operator()(C);
                }

                template <typename T>
                void operator() (T& t)
                {
                    serialization::serialize_tmp_wrapper<T>::deserialize(ptr, t);
                    ptr += serialization::size_c_tmp_wrapper<T>::size(t);
                }

                template <typename T>
                void operator() (const T& t)
                {
                    ptr += serialization::size_c_tmp_wrapper<T>::size(t);
                }

                template <typename ...T>
                void operator() (pattern<T...>& t)
                {
                    sequence_deserializer s(ptr);
                    algorithm::for_each(s, t);
                    ptr = s.ptr;
                }
            };
        };

        template <typename ...Args>
        class size_c_tmp<false, pattern<Args...>>: sequence_serialize
        {
        public:
            static size_t size(const pattern<Args...>& p)
            {
                return calc_sequence_size(p);
            }
        };

        template <typename ...Args>
        class serialize_tmp<false, pattern<Args...>>: sequence_serialize
        {
        public:
            static void serialize(uint8_t *pos, const pattern<Args...>& p)
            {
                sequence_serializer s(pos);
                s(p);
            }

            static void deserialize(const uint8_t *pos, pattern<Args...>& p)
            {
                sequence_deserializer s(pos);
                s(p);
            }
        };
    }

    namespace package_buffer
    {
        template <typename... Args>
        struct compile_time_size_calc_util
        {   
            constexpr static size_t data_size() { return size<Args...>::value; }
        };

        template <typename ...Args>
        class const_buffer: public compile_time_size_calc_util<Args...>
        {
            static const uint8_t data[];

        public:
            const uint8_t* get_data() const { return data; }
        };

        template <typename ...Args>
        struct const_buffer<pattern<Args...>>: public const_buffer<Args...> {};

        template <typename ...Args>
        const uint8_t const_buffer<Args...>::data[] = { Args::value... };

        template <bool is_const_size, typename NonConstArgs, typename Args>
        class buffer;

        template <typename... Args, typename ...NonConstArgs>
        class buffer<false, pattern<NonConstArgs...>, pattern<Args...>>:
            serialization::serializer<pattern<Args...>, pattern<NonConstArgs...>>,
            serialization::sequence_serialize 
        {
            const size_t d_size;
            uint8_t *data;

        public:
            buffer(const NonConstArgs&... args):
                d_size(this->size(args...)),
                data(new uint8_t[d_size])
            {
                this->serialize(data, args...);
            }

            buffer(const pattern<Args...>& p):
                d_size(calc_sequence_size(p)),
                data(new uint8_t[d_size])
            {                
                sequence_serializer s(data);
                s(p);
            }

            ~buffer() { delete []data; }

            size_t data_size() const { return d_size; }
            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args, typename ...NonConstArgs>
        class buffer<true, pattern<NonConstArgs...>, pattern<Args...>>:
            public compile_time_size_calc_util<Args...>,
            serialization::serializer<pattern<Args...>, pattern<NonConstArgs...>>,
            serialization::sequence_serialize 
        {
            uint8_t data[size<Args...>::value];

        public:
            buffer(const NonConstArgs&... args)
            {
                this->serialize(data, args...);
            }

            buffer(const pattern<Args...>& p)
            {                
                sequence_serializer s(data);
                s(p);
            }

            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args>
        class buffer<true, pattern<>, pattern<Args...>>: public const_buffer<metaprogramming::serialize<Args...>>
        {};
    }

    template <typename ...Args>
    using package =
    package_buffer::buffer
    <
        metaprogramming::is_const_size<Args...>::value,
        no_const_args<Args...>,
        Args...
    >;

    namespace package_parser
    { 
        template <typename NonConstArgs, typename Args>
        class parser;

        template <typename... Args, typename ...NonConstArgs>
        class parser<pattern<NonConstArgs...>, pattern<Args...>>:
            serialization::serializer<pattern<Args...>, pattern<NonConstArgs...>>,
            serialization::sequence_serialize 
        {
        public:
            static void parse(const uint8_t* src, NonConstArgs&... args)
            {
                serialization::serializer<pattern<Args...>, pattern<NonConstArgs...>>::deserialize(src, args...);
            }
    
            static void parse(const uint8_t* src, pattern<Args...>& p)
            {                
                sequence_deserializer s(src);
                s(p);
            }
        };
    }

    template <typename ...Args>
    using parser =
    package_parser::parser
    <
        no_const_args<Args...>,
        Args...
    >;
}}

#endif //__PACKAGE__
