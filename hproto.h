#ifndef HPROTO_H
#define HPROTO_H

#include <cstdint>
#include <variant>

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#  error "Big-endian not supported"
#endif

struct HVariant {
    uint64_t id = 0;
    void *data;

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
};
static_assert(sizeof(HVariant) == 16, "Bad struct size");

#endif // HPROTO_H
