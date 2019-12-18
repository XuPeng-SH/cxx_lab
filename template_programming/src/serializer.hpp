#pragma

#include <iostream>
#include "type_traits.hpp"
#include <type_traits>

template <typename T, typename = void>
struct CustomSerializer {
    CustomSerializer() = delete;
};

template <typename T, typename = void>
struct BuiltinSerializer {
    BuiltinSerializer() = delete;
    static void serialize(const T&) {
        static_assert(always_false<T>::value, "T is not serializable");
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

    static void serialize(const T t) {
        std::cout << __func__ << ":" << __LINE__ << " " << t << std::endl;
    }

};

template <typename Arithmetic>
struct BuiltinSerializer<Arithmetic, enable_spec_if<std::is_arithmetic<Arithmetic>>> :
    TrivialSerializer<Arithmetic> {
};

template <>
struct CustomSerializer<const char*> {
    static void serialize(const char* str) {
        std::cout << __func__ << ":" << __LINE__ << " " << str << std::endl;
    }
};

template <typename T>
void Serialize(const T& t) {
    Serializer<T>::type::serialize(t);
}
