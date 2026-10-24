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
struct hproto_traits {};

template <typename T>
    requires std::is_arithmetic_v<T>
struct hproto_traits<T> {
    static constexpr size_t size() {
        return sizeof(T);
    }

    static void write_impl(const T &obj, char **data) {
        std::memcpy(*data, &obj, sizeof(T));
        *data += sizeof(T);
    }

    static T read_impl(const char **data) {
        T obj = *reinterpret_cast<const T*>(*data);
        *data += size();
        return obj;
    }
};

struct AnyType {
    template <typename T> operator T();
};

template <typename T>
    requires std::is_aggregate_v<T>
struct hproto_traits<T> {
    static constexpr size_t size() {
        using Decayed = std::decay_t<T>;

        if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
            static_assert(false, "hproto_write: Type contains too many fields");
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a, b, c, d] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>,
                    std::type_identity<decltype(b)>,
                    std::type_identity<decltype(c)>,
                    std::type_identity<decltype(d)>
                >{};
            }(std::declval<Decayed&>()));

            return hproto_traits<typename std::tuple_element_t<0, Fields>::type>::size()
                 + hproto_traits<typename std::tuple_element_t<1, Fields>::type>::size()
                 + hproto_traits<typename std::tuple_element_t<2, Fields>::type>::size()
                 + hproto_traits<typename std::tuple_element_t<3, Fields>::type>::size();
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a, b, c] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>,
                    std::type_identity<decltype(b)>,
                    std::type_identity<decltype(c)>
                >{};
            }(std::declval<Decayed&>()));

            return hproto_traits<typename std::tuple_element_t<0, Fields>::type>::size()
                 + hproto_traits<typename std::tuple_element_t<1, Fields>::type>::size()
                 + hproto_traits<typename std::tuple_element_t<2, Fields>::type>::size();
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a, b] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>,
                    std::type_identity<decltype(b)>
                >{};
            }(std::declval<Decayed&>()));

            return hproto_traits<typename std::tuple_element_t<0, Fields>::type>::size()
                 + hproto_traits<typename std::tuple_element_t<1, Fields>::type>::size();
        } else if constexpr (requires { Decayed{AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>
                >{};
            }(std::declval<Decayed&>()));

            return hproto_traits<typename std::tuple_element_t<0, Fields>::type>::size();
        }
        return 0;
    }

    static void write_impl(const T &obj, char **data) {
        using Decayed = std::decay_t<T>;

        if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
            static_assert(false, "hproto_write: Type contains too many fields");
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
            auto &&[a, b, c, d] = obj;
            hproto_traits<std::remove_cvref_t<decltype(a)>>::write_impl(a, data);
            hproto_traits<std::remove_cvref_t<decltype(b)>>::write_impl(b, data);
            hproto_traits<std::remove_cvref_t<decltype(c)>>::write_impl(c, data);
            hproto_traits<std::remove_cvref_t<decltype(d)>>::write_impl(d, data);
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}}; }) {
            auto &&[a, b, c] = obj;
            hproto_traits<std::remove_cvref_t<decltype(a)>>::write_impl(a, data);
            hproto_traits<std::remove_cvref_t<decltype(b)>>::write_impl(b, data);
            hproto_traits<std::remove_cvref_t<decltype(c)>>::write_impl(c, data);
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}}; }) {
            auto &&[a, b] = obj;
            hproto_traits<std::remove_cvref_t<decltype(a)>>::write_impl(a, data);
            hproto_traits<std::remove_cvref_t<decltype(b)>>::write_impl(b, data);
        } else if constexpr (requires { Decayed{AnyType{}}; }) {
            auto &&[a] = obj;
            hproto_traits<std::remove_cvref_t<decltype(a)>>::write_impl(a, data);
        }
    }

    static T read_impl(const char **data) {
        using Decayed = std::decay_t<T>;

        if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
            static_assert(false, "hproto_write: Type contains too many fields");
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}, AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a, b, c, d] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>,
                    std::type_identity<decltype(b)>,
                    std::type_identity<decltype(c)>,
                    std::type_identity<decltype(d)>
                >{};
            }(std::declval<Decayed&>()));

            return {hproto_traits<typename std::tuple_element_t<0, Fields>::type>::read_impl(data),
                    hproto_traits<typename std::tuple_element_t<1, Fields>::type>::read_impl(data),
                    hproto_traits<typename std::tuple_element_t<2, Fields>::type>::read_impl(data),
                    hproto_traits<typename std::tuple_element_t<3, Fields>::type>::read_impl(data)};
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}, AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a, b, c] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>,
                    std::type_identity<decltype(b)>,
                    std::type_identity<decltype(c)>
                >{};
            }(std::declval<Decayed&>()));

            return {hproto_traits<typename std::tuple_element_t<0, Fields>::type>::read_impl(data),
                    hproto_traits<typename std::tuple_element_t<1, Fields>::type>::read_impl(data),
                    hproto_traits<typename std::tuple_element_t<2, Fields>::type>::read_impl(data)};
        } else if constexpr (requires { Decayed{AnyType{}, AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a, b] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>,
                    std::type_identity<decltype(b)>
                >{};
            }(std::declval<Decayed&>()));

            return {hproto_traits<typename std::tuple_element_t<0, Fields>::type>::read_impl(data),
                    hproto_traits<typename std::tuple_element_t<1, Fields>::type>::read_impl(data)};
        } else if constexpr (requires { Decayed{AnyType{}}; }) {
            using Fields = decltype([](Decayed& object) {
                auto&& [a] = object;

                return std::tuple<
                    std::type_identity<decltype(a)>
                >{};
            }(std::declval<Decayed&>()));

            return {hproto_traits<typename std::tuple_element_t<0, Fields>::type>::read_impl(data)};
        } else {
            return {};
        }
    }
};

template <typename T, typename... Ts>
bool hproto_try_variant_type(const char **data, hproto_id_t id, std::variant<Ts...> &var) {
    if (id != hproto_id<T>())
        return false;

    var.template emplace<T>(std::move(hproto_traits<T>::read_impl(data)));

    return true;
}

template<typename... Ts>
struct hproto_traits<std::variant<Ts...>> {
    static constexpr size_t size() {
        return sizeof(hproto_id_t) +
               std::max({ hproto_traits<std::remove_cvref_t<Ts>>::size()... });
    }

    static void write_impl(const std::variant<Ts...> &var, char **data) {
        std::visit([data](const auto& value) {
            hproto_id_t id = hproto_id<std::remove_cvref_t<decltype(value)>>();
            hproto_traits<hproto_id_t>::write_impl(id, data);
            hproto_traits<std::remove_cvref_t<decltype(value)>>::write_impl(value, data);
        }, var);
    }

    static std::variant<Ts...> read_impl(const char **data) {
        hproto_id_t id = hproto_traits<hproto_id_t>::read_impl(data);
        std::variant<Ts...> var;

        (hproto_try_variant_type<Ts, Ts...>(data, id, var) || ...);

        return var;
    }
};

template<typename T>
concept hproto_serializable = requires(const T& value) {
    { hproto_traits<T>::size() } -> std::same_as<size_t>;
    { hproto_traits<T>::write_impl(value, nullptr) } -> std::same_as<void>;
    { hproto_traits<T>::read_impl(nullptr) } -> std::same_as<T>;
};

template<hproto_serializable T>
void hproto_write(const T &obj, char *data) {
    hproto_traits<T>::write_impl(obj, &data);
}

template<hproto_serializable T>
T hproto_read(const char *data) {
    return hproto_traits<T>::read_impl(&data);
}

template<hproto_serializable T>
size_t hproto_size(const T &obj) {
    return hproto_traits<T>::size();
}
