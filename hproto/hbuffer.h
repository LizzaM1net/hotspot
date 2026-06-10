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
void write_blob(char*& dst, const T& val) {
    h_size_t n = HBufferTraits<T>::size(val);
    memcpy(dst, &n, sizeof(n));
    dst += sizeof(n);
    memcpy(dst, HBufferTraits<T>::data(val), n);
    dst += n;
}

template<typename T>
T read_blob(const char*& ptr) {
    h_size_t n;
    memcpy(&n, ptr, sizeof(n));
    ptr += sizeof(n);
    T result = HBufferTraits<T>::from_blob(ptr, n);
    ptr += n;
    return result;
}

template<>
struct HProtoData<std::string> {
    static constexpr const hproto_id_t hproto_id = 0x0002;
    static constexpr size_t hproto_size(const std::string &s) {
        return sizeof(h_size_t) + s.size();
    }
    static bool hproto_accepts_size(size_t s) {
        // TODO: Maybe move size validation to read
        return s >= sizeof(h_size_t);
    }
    static void hproto_write(const std::string &s, char* data) {
        write_blob(data, s);
    }
    static std::string hproto_read(const char* data) {
        return read_blob<std::string>(data);
    }
};
