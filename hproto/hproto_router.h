#pragma once

#include "hproto.h"
#include "hbuffer.h"
#include "hproto_types.h"

#include <vector>

struct RouterCreateWaitroomRequest {
    HSocketAddress localAddress;
};

struct RouterRedirectAnswer {
    HSocketAddress peerAddress;
};

struct RouterGreet {};

struct HotspotFile {
    HotspotFile(std::string n, std::vector<char> d)
        : name(n)
        , data(d) {}

    HotspotFile() {}

    std::string name;
    std::vector<char> data;
};

template <>
constexpr std::string_view hproto_name<HotspotFile>() {
    return "HotspotFile";
}

template <>
constexpr size_t hproto_size<HotspotFile>() {
    return sizeof(h_size_t) + sizeof(h_size_t) + 1000;
}

template <>
void hproto_write_impl(const HotspotFile &f, char **data) {
    write_blob(data, f.name);
    write_blob(data, f.data);
}

template <>
HotspotFile hproto_read_impl<HotspotFile>(const char **data) {
    HotspotFile file;
    file.name = read_blob<std::string>(data);
    file.data = read_blob<std::vector<char>>(data);
    return file;
}
