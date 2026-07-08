#include <cstddef>
#include <string_view>

typedef uint64_t hproto_id_t;

// Idea kindly borrowed from https://github.com/TheLartians/StaticTypeInfo
namespace hproto_reflection {
    template <typename T>
    constexpr std::string_view template_signature() {
        return __PRETTY_FUNCTION__;
    }
    constexpr static size_t prefixSize = template_signature<double>().find("double");
    constexpr static size_t suffixSize = template_signature<double>().size() - prefixSize - sizeof("double") + 1;
}
template <typename T>
constexpr std::string_view hproto_type_name() {
    std::string_view view = hproto_reflection::template_signature<T>();
    view.remove_prefix(hproto_reflection::prefixSize);
    view.remove_suffix(hproto_reflection::suffixSize);
    return view;
}

constexpr hproto_id_t fnv1a_hash(const char *data, size_t size) {
    hproto_id_t hash = 0xcbf29ce484222325;

    for (size_t i = 0; i < size; i++) {
        hash ^= static_cast<uint8_t>(data[i]);
        hash *= 0x00000100000001b3;
    }

    return hash;
}

template <typename T>
struct HProtoId {
    static constexpr std::string_view name = hproto_type_name<T>();
    static constexpr hproto_id_t id = fnv1a_hash(name.data(), name.size());
};
