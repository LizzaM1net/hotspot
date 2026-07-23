#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <variant>

#include "hproto_reflection.h"
#include "hlog.h"

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Big-endian not supported"
#endif

typedef uint64_t h_size_t;
typedef uint64_t h_offset_t;

template <typename T>
struct HProtoData {};

template <typename T>
void hproto_write(const T &obj, char **data);

template <typename T>
T hproto_read(const char **data);

template <typename T>
constexpr size_t hproto_size();

template <typename T>
requires std::is_aggregate_v<T> || std::is_arithmetic_v<T>
constexpr size_t hproto_size() {
    return sizeof(T);
}

struct AnyType {
    template <typename T> operator T();
};

template <typename T>
requires std::is_aggregate_v<T>
void hproto_write(const T &obj, char **data) {
    using Decayed = std::decay_t<T>;
    std::memset(*data, 0, sizeof(T));

    if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
        static_assert(false, "hproto_write: Type contains too many fields");
    } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
        auto&& [a, b, c, d] = obj;
        std::construct_at((T*)(*data), a, b, c, d);
    } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}}; }) {
        auto&& [a, b, c] = obj;
        std::construct_at((T*)(*data), a, b, c);
    } else if constexpr (requires { Decayed{AnyType{}, AnyType{}}; }) {
        auto&& [a, b] = obj;
        std::construct_at((T*)(*data), a, b);
    } else if constexpr (requires { Decayed{AnyType{}}; }) {
        auto&& [a] = obj;
        std::construct_at((T*)(*data), a);
    }
    *data += sizeof(T);
}

template <typename T>
requires std::is_arithmetic_v<T>
void hproto_write(const T &obj, char **data) {
    std::memcpy(*data, &obj, sizeof(T));
    *data += sizeof(T);
}

template <typename T>
requires std::is_aggregate_v<T> || std::is_arithmetic_v<T>
T hproto_read(const char **data) {
    T obj = *reinterpret_cast<const T*>(*data);
    *data += hproto_size<T>();
    return obj;
}

template <typename Ts>
constexpr size_t hproto_size(std::variant<Ts> variant) {
    return std::visit([](const auto& value) {
        return sizeof(hproto_id_t) + hproto_size<std::remove_cvref_t<decltype(value)>>();
    }, variant);
}

template <typename Args>
void hproto_write(std::variant<Args> variant, char **data) {
    std::visit([data](const auto& value) {
        hproto_id_t id = hproto_id<std::remove_cvref_t<decltype(value)>>();
        memcpy(*data, &id, sizeof(hproto_id_t));
        *data += sizeof(hproto_id_t);
        hproto_write(value, data);
    }, variant);
}

template <typename T, typename... Ts>
bool hproto_try_variant_type(const char **data, size_t size, hproto_id_t id, std::variant<Ts...> &var) {
    if (id != hproto_id<T>())
        return false;

    var.template emplace<T>(std::move(hproto_read<T>(data)));

    return true;
}

template <typename... Ts>
std::variant<std::monostate, Ts...> hproto_read(const char **data, size_t size) {
    hproto_id_t id;
    memcpy(&id, *data, sizeof(hproto_id_t));
    *data += sizeof(hproto_id_t);
    std::variant<std::monostate, Ts...> var;

    (hproto_try_variant_type<Ts, std::monostate, Ts...>(data, size-sizeof(hproto_id_t), id, var) || ...);

    return var;
}
