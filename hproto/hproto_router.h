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
struct hproto_traits<HotspotFile> {
    static constexpr size_t size() {
        return sizeof(h_size_t) + sizeof(h_size_t) + 1000;
    }

    static void write_impl(const HotspotFile &f, char **data) {
        write_blob(data, f.name);
        write_blob(data, f.data);
    }

    static HotspotFile read_impl(const char **data) {
        HotspotFile file;
        file.name = read_blob<std::string>(data);
        file.data = read_blob<std::vector<char>>(data);
        return file;
    }
};
