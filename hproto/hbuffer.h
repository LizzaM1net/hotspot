#pragma once

#include "hproto.h"

#include <cstddef>
#include <string>
#include <vector>

template <typename T>
struct HBufferTraits {
    static const char *data(const T&);
    static size_t size(const T&);
    static T from_blob(const char *data, size_t size);
};

template <>
struct HBufferTraits<std::string> {
    static const char* data(const std::string& s) { return s.data(); }
    static size_t size(const std::string& s) { return s.size(); }
    static std::string from_blob(const char* ptr, size_t len) { return {ptr, len}; }
};

template <>
struct HBufferTraits<std::vector<char>> {
    static const char* data(const std::vector<char>& s) { return s.data(); }
    static size_t size(const std::vector<char>& s) { return s.size(); }
    static std::vector<char> from_blob(const char* ptr, size_t len) { return {ptr, ptr+len}; }
};

template<typename T>
void write_blob(char **data, const T& val) {
    h_size_t n = HBufferTraits<T>::size(val);
    memcpy(*data, &n, sizeof(n));
    *data += sizeof(n);
    memcpy(*data, HBufferTraits<T>::data(val), n);
    *data += n;
}

template<typename T>
T read_blob(const char **ptr) {
    h_size_t n;
    memcpy(&n, *ptr, sizeof(n));
    *ptr += sizeof(n);
    T result = HBufferTraits<T>::from_blob(*ptr, n);
    *ptr += n;
    return result;
}

template<>
constexpr size_t hproto_size<std::string>() {
    return sizeof(h_size_t) + sizeof(h_offset_t);
}

template <>
void hproto_write_impl<std::string>(const std::string &s, char **data) {
    write_blob(data, s);
}

template <>
std::string hproto_read_impl<std::string>(const char **data) {
    return read_blob<std::string>(data);
}

template <>
constexpr std::string_view hproto_name<std::string>() {
    return "HString";
}
