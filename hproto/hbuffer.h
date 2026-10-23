#pragma once

#include "hproto.h"

#include <cstddef>
#include <string>
#include <vector>

template <>
struct hproto_traits<std::vector<char>> {
    static constexpr size_t size() {
        return sizeof(h_size_t) + 1000;
    }

    static void write_impl(const std::vector<char> &c, char **data) {
        h_size_t n = c.size();
        hproto_traits<h_size_t>::write_impl(n, data);
        memcpy(*data, c.data(), n);
        *data += n;
    }

    static std::vector<char> read_impl(const char **data) {
        h_size_t n = hproto_traits<h_size_t>::read_impl(data);
        std::vector<char> result(*data, *data+n);
        *data += n;
        return result;
    }
};

template <>
struct hproto_traits<std::string> {
    static constexpr size_t size() {
        return sizeof(h_size_t) + sizeof(h_offset_t);
    }

    static void write_impl(const std::string &s, char **data) {
        h_size_t n = s.size();
        hproto_traits<h_size_t>::write_impl(n, data);
        memcpy(*data, s.data(), n);
        *data += n;
    }

    static std::string read_impl(const char **data) {
        h_size_t n = hproto_traits<h_size_t>::read_impl(data);
        std::string result(*data, n);
        *data += n;
        return result;
    }
};

template <>
constexpr std::string_view hproto_name<std::string>() {
    return "HString";
}
