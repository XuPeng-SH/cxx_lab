#pragma once

#include <string>
/* #include <rocksdb/slice.h> */

namespace util {

template <typename...> using void_t = void;

template <typename Pred>
using enable_spec_if = void_t<std::enable_if<Pred::value, void>>;

template <typename>
struct always_false : std::false_type::type {};

template <typename T>
class HasSize {
    using one = char;
    struct two { char x[2]; };
    template <typename C> static one test(typeof(&C::size));
    template <typename C> static two test(...);
public:
    static constexpr const bool value = sizeof(test<T>(0)) == sizeof(char);
    /* enum { value = sizeof(test<T>(0)) == sizeof(char) }; */
};

template <typename T, typename = void>
struct CustomSerializer {
    CustomSerializer() = delete;
};

template<typename T>
struct CustomSerializer<T, enable_spec_if<HasSize<T>>> {
    static void Serialize(const T& t) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl;
        /* std::cout << __FILE__ << ":" << __LINE__ << " " << t.data() << std::endl; */
    }
};

/* template<typename T> */
/* struct CustomSerializer<T, enable_spec_if<HasSize<T>>> { */
/*     static void Serialize(const T& t) { */
/*         std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl; */
/*         /1* std::cout << __FILE__ << ":" << __LINE__ << " " << t.data() << std::endl; *1/ */
/*     } */
/* }; */

template <typename T, typename = void>
struct BuiltinSerializer {
    BuiltinSerializer() = delete;
    static void Serialize(const T&) {
        static_assert(always_false<T>::value, "Not Serializable");
    }
};

template <typename T>
struct Serializer {
    using type = std::conditional_t<std::is_constructible<CustomSerializer<T>>::value,
          CustomSerializer<T>,
          BuiltinSerializer<T>
    >;
};

template <typename T>
struct TrivialSerializer {
    static_assert(std::is_trivially_copyable<T>::value, "");

    static void Serialize(const T& t) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << t << std::endl;
    }
};

template <typename Arithmetic>
struct BuiltinSerializer<Arithmetic, enable_spec_if<std::is_arithmetic<Arithmetic>>> :
    TrivialSerializer<Arithmetic> {
};

template <>
struct CustomSerializer<const char*> {
    static void Serialize(const char* str) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << str << std::endl;
    }
};

template <typename T>
void Serialize(const T& t) {
    Serializer<T>::type::Serialize(t);
}

}
