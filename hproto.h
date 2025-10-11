#ifndef HPROTO_H
#define HPROTO_H

#include <cstdint>
#include <variant>
#include <format>

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Big-endian not supported"
#endif

typedef uint32_t hproto_id_t;

template <typename T>
struct HProtoData {
    static constexpr const hproto_id_t id = 0;
    static constexpr size_t hproto_size = 0;
};

#define HOTSPOT_SIZED_OBJECT(name, id, size) static_assert(sizeof(name) == size, "Bad hotspot type size: "#name);\
template<>\
struct HProtoData<name> {\
    static constexpr const hproto_id_t hproto_id = id;\
    static constexpr const size_t hproto_size = size;\
};

template <typename Ts>
size_t hproto_size(std::variant<Ts> variant) {
    return sizeof(hproto_id_t)+std::visit([](const auto& value) { return sizeof(value); }, variant);
}

template <typename Ts>
void hproto_write(std::variant<Ts> variant, void *data) {
    std::visit([data](const auto& value) {
        hproto_id_t id = HProtoData<std::remove_cvref_t<decltype(value)>>::hproto_id;
        memcpy(data, &id, sizeof(hproto_id_t));
        memcpy((char*)data+sizeof(hproto_id_t), &value, sizeof(value));
    }, variant);
}

template <typename T, typename... Ts>
bool hproto_try_variant_type(void *data, size_t size, hproto_id_t id, std::variant<Ts...> &var) {
    if (id != HProtoData<T>::hproto_id || size != HProtoData<T>::hproto_size)
        return false;

    var.template emplace<T>(*reinterpret_cast<T *>(data));

    return true;
}

template <typename... Ts>
std::variant<std::monostate, Ts...> hproto_read(void *data, size_t size) {
    hproto_id_t id;
    memcpy(&id, data, sizeof(hproto_id_t));
    std::variant<std::monostate, Ts...> var;

    (hproto_try_variant_type<Ts, std::monostate, Ts...>((char*)data+sizeof(hproto_id_t), size-sizeof(hproto_id_t), id, var) || ...);

    return var;
}

#endif // HPROTO_H
