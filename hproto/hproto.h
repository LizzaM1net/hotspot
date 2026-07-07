#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <variant>

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Big-endian not supported"
#endif

typedef uint32_t hproto_id_t;
typedef uint64_t h_size_t;

template <typename T>
struct HProtoData {};

template <typename T>
void hproto_write(const T &obj, char *data);

template <typename T>
T hproto_read(const char* data);

struct AnyType {
    template <typename T> operator T();
};

template <typename T>
requires std::is_aggregate_v<T>
void hproto_write(const T &obj, char *data) {
    using Decayed = std::decay_t<T>;
    std::memset(data, 0, sizeof(T));

    if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
        static_assert(false, "hproto_write: Type contains too many fields");
    } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
        auto&& [a, b, c, d] = obj;
        std::construct_at((T*)data, a, b, c, d);
    } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}}; }) {
        auto&& [a, b, c] = obj;
        std::construct_at((T*)data, a, b, c);
    } else if constexpr (requires { Decayed{AnyType{}, AnyType{}}; }) {
        auto&& [a, b] = obj;
        std::construct_at((T*)data, a, b);
    } else if constexpr (requires { Decayed{AnyType{}}; }) {
        auto&& [a] = obj;
        std::construct_at((T*)data, a);
    }
}

template <typename T>
requires std::is_aggregate_v<T>
T hproto_read(const char* data) {
    return *reinterpret_cast<const T*>(data);
}

#define HOTSPOT_SIZED_OBJECT(name, id, size)\
static_assert(sizeof(name) == size, "Bad hotspot type size: "#name);\
template<>\
struct HProtoData<name> {\
    static constexpr const hproto_id_t hproto_id = id;\
    static constexpr size_t hproto_size(const name &) {\
        return size;\
    }\
    static bool hproto_accepts_size(size_t s) {\
        return s == size;\
    }\
};

#define HOTSPOT_EMPTY_OBJECT(name, id)\
template<>\
struct HProtoData<name> {\
    static constexpr const hproto_id_t hproto_id = id;\
    static constexpr size_t hproto_size(const name &) {\
        return 0;\
    }\
    static bool hproto_accepts_size(size_t s) {\
        return s == 0;\
    }\
};

template <typename Ts>
size_t hproto_size(std::variant<Ts> variant) {
    return std::visit([](const auto& value) {
        return sizeof(hproto_id_t) + HProtoData<std::remove_cvref_t<decltype(value)>>::hproto_size(value);
    }, variant);
}

template <typename Args>
void hproto_write(std::variant<Args> variant, char *data) {
    std::visit([data](const auto& value) {
        hproto_id_t id = HProtoData<std::remove_cvref_t<decltype(value)>>::hproto_id;
        memcpy(data, &id, sizeof(hproto_id_t));
        hproto_write(value, data+sizeof(hproto_id_t));
    }, variant);
}

template <typename T, typename... Ts>
bool hproto_try_variant_type(const char *data, size_t size, hproto_id_t id, std::variant<Ts...> &var) {
    if (id != HProtoData<T>::hproto_id)
        return false;

    if (!HProtoData<T>::hproto_accepts_size(size))
        return false;

    var.template emplace<T>(std::move(hproto_read<T>(data)));

    return true;
}

template <typename... Ts>
std::variant<std::monostate, Ts...> hproto_read(const char *data, size_t size) {
    hproto_id_t id;
    memcpy(&id, data, sizeof(hproto_id_t));
    std::variant<std::monostate, Ts...> var;

    (hproto_try_variant_type<Ts, std::monostate, Ts...>(data+sizeof(hproto_id_t), size-sizeof(hproto_id_t), id, var) || ...);

    return var;
}
