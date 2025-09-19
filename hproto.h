#ifndef HPROTO_H
#define HPROTO_H

#include <cstdint>
#include <variant>
#include <format>

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Big-endian not supported"
#endif

#define H_SIZED_OBJECT(id) static constexpr uint32_t hproto_id = id;\
static size_t hproto_size;

#define HOTSPOT_GUARD_SIZE(name, size) static_assert(sizeof(name) == size, "Bad hotspot type size: "#name);\
size_t name::hproto_size = size;

struct HVariant {
    uint64_t id = 0;
    void *data = nullptr;

    template<typename T>
    void operator=(T& t) {
        data = malloc(sizeof(T));
        memcpy(data, &t, sizeof(T));
        id = T::hproto_id;
    }

    template<typename T>
    T* value() {
        if (id != T::hproto_id)
            return nullptr;

        return (T*)data;
    }

    template<typename T>
    bool holds_alternative() {
        return T::hproto_id == id;
    }

    template<typename T>
    size_t serializedSize() {
        return sizeof(uint64_t)+sizeof(T);
    }

    template<typename T>
    void serializeToArray(void* array) {
        if (array == nullptr || data == nullptr)
            return;

        memcpy(array, &id, sizeof(uint64_t));
        memcpy((char*)array+sizeof(uint64_t), data, sizeof(T));
    }

    template<typename T>
    static bool try_cast(HVariant &variant, void* array, size_t size) {
        if (size < sizeof(uint32_t) + sizeof(T) || array == nullptr)
            return false;

        memcpy(&variant.id, array, sizeof(uint64_t));

        if (T::hproto_id != variant.id)
            return false;

        variant.data = malloc(sizeof(T));
        memcpy(variant.data, (char*)array+sizeof(uint64_t), sizeof(T));
        return true;
    }

    template<typename... Ts>
    static HVariant deserializeFromArray(void* array, size_t size) {
        HVariant variant;
        (try_cast<Ts>(variant, array, size) || ...);
        return variant;
    }

    template<typename T>
    T cast() {
        if (std::remove_pointer_t<T>::hproto_id != id)
            return nullptr;

        return static_cast<T>(data);
    }
};

#endif // HPROTO_H
